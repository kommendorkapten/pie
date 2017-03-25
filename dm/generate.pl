#!/usr/bin/perl

##
# Copyright (C) 2016 Fredrik Skogman, skogman - at - gmail.com.
# This file is part of pie project
#
# The contents of this file are subject to the terms of the Common
# Development and Distribution License (the "License"). You may not use this
# file except in compliance with the License. You can obtain a copy of the
# License at http://opensource.org/licenses/CDDL-1.0. See the License for the
# specific language governing permissions and limitations under the License. 
# When distributing the software, include this License Header Notice in each
# file and include the License file at http://opensource.org/licenses/CDDL-1.0.
##

#
# Very simlple and crude C code generator for interfacing with SQLite.
# SQL parser is ad-hoc, and may not work with all cases. Parser is line-based,
# and hence the CREATE TABLE xxx part must be on a seprarate line.
# Each .sql file may only contain a single table definition.
# Input is one or more sql files, output is one .h/.c for each table CRUD
# (Create, Read, Update, Delete) functions are created.
# Possible extensions in the future is to add multiple database engines,
# such as MySQl and support to generate for more languages.
#

use warnings;
use strict;    

my %data_types = (
    "CHARACTER"         => "char*",
    "VARCHAR"           => "char*",
    "CHARACTER VARYING" => "char*",
    "BOOLEAN"           => "char",
    "SMALLINT"          => "short",
    "INTEGER"           => "int",
    "INT"               => "int",    
    "BIGINT"            => "long",
    "REAL"              => "float", 
    "FLOAT"             => "float",
    "DOUBLE PRECISION"  => "double",
    "DATE"              => "long",
    "TIME"              => "long",
    "TIMESTAMP"         => "long",
    );

my $table_def = 'CREATE\s+TABLE\s+(\w+)\s*\(';
my $column_def = '(\w+)\s+(\w+)';
my $primary_key_def = 'PRIMARY\s+KEY\(([^\)]+)\)';

my $db_ref_name = "db";
my $this = "this";

# Sqlite
my $db_ref_type = "sqlite3*";

# Globals
my @structs;
my $fhh;
my $fhc;

foreach my $file (@ARGV) {
    my $lines = get_content($file);
    my $struct = {'filename' => $file};
    
    foreach my $line (@$lines) {
        if ($line =~ /$table_def/) {
            my $name = $1;
            $$struct{'name'} = "$name";
            next;
        }
        if ($line =~ /$column_def/) {
            my $name = $1;
            my $type = $2;
            my $ctype = $data_types{$type};

            if (defined $ctype) {
                push @{$$struct{'variables'}}, {'name' => $name, 
                                                'sqltype' => $type,
                                                'ctype' => $ctype,};
                next;
            }
        }
        if ($line =~ /$primary_key_def/) {
            my $tmp = $1;
            $tmp =~ s/\s+//g;
            my @pk = split /,/, $tmp;
            $$struct{'primary_key'} = \@pk;
        }
    }

    push @structs, $struct;
}

foreach my $struct (@structs) {
    my $h_filename;
    my $c_filename;
    my $filename = $$struct{'filename'};
    my $now = localtime();
    my $guard;

    $filename =~ s/\.sql$//;

    $h_filename = "$filename.h";
    $c_filename = "$filename.c";
    $guard = uc "__" . $filename . "_h__";
    
    open($fhh, ">", $h_filename) or die "Could not create $h_filename";
    open($fhc, ">", $c_filename) or die "Could not create $c_filename";

    
    print $fhh "/* Automatically generated at $now */\n";
    print $fhc "/* Automatically generated at $now */\n";    

    print $fhh "#ifndef $guard\n";
    print $fhh "#define $guard\n";    
    print $fhh "#include <sqlite3.h>\n";        
    print $fhc "#include <stddef.h>\n";
    print $fhc "#include \"$h_filename\"\n";

    print $fhh "struct $$struct{'name'}\n";
    print $fhh "{\n";
    foreach my $var (@{$$struct{'variables'}}) {
        print $fhh "$$var{ctype} $$var{name};\n";
    }
    print $fhh "};\n";
    generate_delete($struct);

    print $fhh "#endif /* $guard */\n";
    
    close $fhh;
    close $fhc;

    `indent -bl -bap -bad $h_filename`;
    `indent -bl -bap -bad $c_filename`;    
}

sub get_content {
    my ($file) = @_;
    my @content;
    
    open(my $fh, $file) or die "Could not open '$file'";

    while(my $line = <$fh>) {
        chomp $line;
        my @comp;

        if ($line =~ /PRIMARY\s+KEY/) {
            push @comp, $line;
        } else {
            @comp = split /,/, $line;   
        }
        push @content, @comp;
    }
    
    close $fh;

    return \@content
}

sub generate_delete {
    my ($struct) = @_;
    my $q = "DELETE FROM $$struct{'name'} WHERE ";
    my $pk = $$struct{'primary_key'};
    my $num_pk = scalar @$pk;

    $q = $q . "$$pk[0] = ?";

    for (my $i = 1; $i < $num_pk; $i++) {
        $q = $q . " AND $$pk[$i] = ?";
    }

    # Prologue
    print $fhh "extern int $$struct{'name'}_delete($db_ref_type $db_ref_name, struct $$struct{'name'}* $this);";
    print $fhc "int $$struct{'name'}_delete($db_ref_type $db_ref_name, struct $$struct{'name'}* $this) {\n";
    print $fhc "char* q = \"$q\";\n";
    print $fhc "sqlite3_stmt* pstmt;\n";
    print $fhc "int ret;\n";

    # Body
    sqlite_call("sqlite3_prepare_v2", $db_ref_name, , "q", -1, "&pstmt", "NULL");
    for (my $i = 0; $i < $num_pk; $i++) {
        # all binds occur at index 1
        sqlite_bind($struct, $i + 1, $$pk[$i]);
    }

    sqlite_call_ex("sqlite3_step", "SQLITE_DONE", "pstmt");
    
    # Epiloguqe
    print $fhc "ret = 0;\n";
    print $fhc "cleanup:\n";
    sqlite_call_last("sqlite3_finalize", "pstmt");
    print $fhc "return ret;\n";
    print $fhc "}\n";
}

sub sqlite_call {
    my $fun = shift @_;
    my @args = @_;
    my $num_args = scalar @args;

    print $fhc "ret = $fun($args[0]";
    for (my $i = 1; $i < $num_args; $i++) {
        print $fhc ",$args[$i]";
    }
    print $fhc ");\n";
    print $fhc "if (ret != SQLITE_OK) {ret = -1; goto cleanup;}\n";
}

sub sqlite_call_last {
    my $fun = shift @_;
    my @args = @_;
    my $num_args = scalar @args;

    print $fhc "ret = $fun($args[0]";
    for (my $i = 1; $i < $num_args; $i++) {
        print $fhc ",$args[$i]";
    }
    print $fhc ");\n";
    print $fhc "if (ret != SQLITE_OK) {ret = -1;}\n";
}

sub sqlite_call_ex {
    my $fun = shift @_;
    my $ret = shift @_;
    my @args = @_;
    my $num_args = scalar @args;

    print $fhc "ret = $fun($args[0]";
    for (my $i = 1; $i < $num_args; $i++) {
        print $fhc ",$args[$i]";
    }
    print $fhc ");\n";
    print $fhc "if (ret != $ret) {ret = -1; goto cleanup;}\n";
}

sub sqlite_bind {
    my ($struct, $index, $var) = @_;
    my $ctype = get_ctype($struct, $var);

    if (($ctype eq "char") ||
        ($ctype eq "short") ||
        ($ctype eq "int")) {
        sqlite_call("sqlite3_bind_int", "pstmt", $index, "(int)$this" . "->$var");
    } elsif ($ctype eq "long") {
        sqlite_call("sqlite3_bind_int64", "pstmt", $index, "$this" . "->$var");
    } elsif ($ctype eq "float") {
        sqlite_call("sqlite3_bind_double", "pstmt", $index, "(double)$this" . "->$var");
    } elsif ($ctype eq "double") {
        sqlite_call("sqlite3_bind_double", "pstmt", $index, "$this" . "->$var");
    } elsif ($ctype eq "char*"){
        sqlite_call("sqlite3_bind_text", "pstmt", $index, "$this" . "->$var", -1, "SQLITE_STATIC");
    } else {
        print "Unknown type '$ctype'\n";
        exit(1);
    }
}

sub get_ctype {
    my ($struct, $cvar) = @_;
    my $ctype;

    foreach my $var (@{$$struct{"variables"}}) {
        if ($cvar eq $$var{"name"}) {
            $ctype = $$var{"ctype"};
            last;
        }
    }

    return $ctype;
}
