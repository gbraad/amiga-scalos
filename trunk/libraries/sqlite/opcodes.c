/* Automatically generated.  Do not edit */
/* See the mkopcodec.awk script for details. */
#if !defined(SQLITE_OMIT_EXPLAIN) || !defined(NDEBUG) || defined(VDBE_PROFILE) || defined(SQLITE_DEBUG)
const char *sqlite3OpcodeName(int i){
 static const char *const azName[] = { "?",
     /*   1 */ "VFilter",
     /*   2 */ "IfNeg",
     /*   3 */ "IntegrityCk",
     /*   4 */ "DropTrigger",
     /*   5 */ "DropIndex",
     /*   6 */ "IdxInsert",
     /*   7 */ "Delete",
     /*   8 */ "SeekLt",
     /*   9 */ "OpenEphemeral",
     /*  10 */ "VerifyCookie",
     /*  11 */ "Blob",
     /*  12 */ "RowKey",
     /*  13 */ "IsUnique",
     /*  14 */ "VUpdate",
     /*  15 */ "Expire",
     /*  16 */ "NullRow",
     /*  17 */ "OpenPseudo",
     /*  18 */ "OpenWrite",
     /*  19 */ "Not",
     /*  20 */ "OpenRead",
     /*  21 */ "Transaction",
     /*  22 */ "AutoCommit",
     /*  23 */ "Copy",
     /*  24 */ "Halt",
     /*  25 */ "VRename",
     /*  26 */ "Vacuum",
     /*  27 */ "RowData",
     /*  28 */ "NotExists",
     /*  29 */ "SetCookie",
     /*  30 */ "Move",
     /*  31 */ "Variable",
     /*  32 */ "Pagecount",
     /*  33 */ "VNext",
     /*  34 */ "VDestroy",
     /*  35 */ "TableLock",
     /*  36 */ "RowSetAdd",
     /*  37 */ "LoadAnalysis",
     /*  38 */ "IdxDelete",
     /*  39 */ "Sort",
     /*  40 */ "ResetCount",
     /*  41 */ "Count",
     /*  42 */ "Integer",
     /*  43 */ "Explain",
     /*  44 */ "IncrVacuum",
     /*  45 */ "AggStep",
     /*  46 */ "CreateIndex",
     /*  47 */ "NewRowid",
     /*  48 */ "Return",
     /*  49 */ "Trace",
     /*  50 */ "IfPos",
     /*  51 */ "IdxLT",
     /*  52 */ "Rewind",
     /*  53 */ "SeekGe",
     /*  54 */ "Affinity",
     /*  55 */ "AddImm",
     /*  56 */ "Null",
     /*  57 */ "VColumn",
     /*  58 */ "Clear",
     /*  59 */ "If",
     /*  60 */ "Permutation",
     /*  61 */ "RealAffinity",
     /*  62 */ "HaltIfNull",
     /*  63 */ "Yield",
     /*  64 */ "AggFinal",
     /*  65 */ "IfZero",
     /*  66 */ "Or",
     /*  67 */ "And",
     /*  68 */ "RowSetTest",
     /*  69 */ "Last",
     /*  70 */ "Rowid",
     /*  71 */ "IsNull",
     /*  72 */ "NotNull",
     /*  73 */ "Ne",
     /*  74 */ "Eq",
     /*  75 */ "Gt",
     /*  76 */ "Le",
     /*  77 */ "Lt",
     /*  78 */ "Ge",
     /*  79 */ "Sequence",
     /*  80 */ "BitAnd",
     /*  81 */ "BitOr",
     /*  82 */ "ShiftLeft",
     /*  83 */ "ShiftRight",
     /*  84 */ "Add",
     /*  85 */ "Subtract",
     /*  86 */ "Multiply",
     /*  87 */ "Divide",
     /*  88 */ "Remainder",
     /*  89 */ "Concat",
     /*  90 */ "NotFound",
     /*  91 */ "SeekGt",
     /*  92 */ "MakeRecord",
     /*  93 */ "BitNot",
     /*  94 */ "String8",
     /*  95 */ "ResultRow",
     /*  96 */ "String",
     /*  97 */ "Goto",
     /*  98 */ "Noop",
     /*  99 */ "VCreate",
     /* 100 */ "Program",
     /* 101 */ "RowSetRead",
     /* 102 */ "DropTable",
     /* 103 */ "IdxRowid",
     /* 104 */ "Insert",
     /* 105 */ "Column",
     /* 106 */ "Compare",
     /* 107 */ "VOpen",
     /* 108 */ "Param",
     /* 109 */ "CreateTable",
     /* 110 */ "Found",
     /* 111 */ "Seek",
     /* 112 */ "Close",
     /* 113 */ "Savepoint",
     /* 114 */ "IfNot",
     /* 115 */ "VBegin",
     /* 116 */ "MemMax",
     /* 117 */ "Next",
     /* 118 */ "Prev",
     /* 119 */ "SeekLe",
     /* 120 */ "MustBeInt",
     /* 121 */ "CollSeq",
     /* 122 */ "Gosub",
     /* 123 */ "ParseSchema",
     /* 124 */ "Destroy",
     /* 125 */ "IdxGE",
     /* 126 */ "ReadCookie",
     /* 127 */ "Jump",
     /* 128 */ "Function",
     /* 129 */ "SCopy",
     /* 130 */ "Real",
     /* 131 */ "Int64",
     /* 132 */ "NotUsed_132",
     /* 133 */ "NotUsed_133",
     /* 134 */ "NotUsed_134",
     /* 135 */ "NotUsed_135",
     /* 136 */ "NotUsed_136",
     /* 137 */ "NotUsed_137",
     /* 138 */ "NotUsed_138",
     /* 139 */ "NotUsed_139",
     /* 140 */ "NotUsed_140",
     /* 141 */ "ToText",
     /* 142 */ "ToBlob",
     /* 143 */ "ToNumeric",
     /* 144 */ "ToInt",
     /* 145 */ "ToReal",
  };
  return azName[i];
}
#endif
