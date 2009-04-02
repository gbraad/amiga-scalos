/* Automatically generated.  Do not edit */
/* See the mkopcodec.awk script for details. */
#if !defined(SQLITE_OMIT_EXPLAIN) || !defined(NDEBUG) || defined(VDBE_PROFILE) || defined(SQLITE_DEBUG)
const char *sqlite3OpcodeName(int i){
 static const char *const azName[] = { "?",
     /*   1 */ "VRowid",
     /*   2 */ "VFilter",
     /*   3 */ "IfNeg",
     /*   4 */ "ContextPop",
     /*   5 */ "IntegrityCk",
     /*   6 */ "DropTrigger",
     /*   7 */ "DropIndex",
     /*   8 */ "IdxInsert",
     /*   9 */ "Delete",
     /*  10 */ "SeekLt",
     /*  11 */ "OpenEphemeral",
     /*  12 */ "VerifyCookie",
     /*  13 */ "Blob",
     /*  14 */ "RowKey",
     /*  15 */ "IsUnique",
     /*  16 */ "SetNumColumns",
     /*  17 */ "VUpdate",
     /*  18 */ "Expire",
     /*  19 */ "Not",
     /*  20 */ "NullRow",
     /*  21 */ "OpenPseudo",
     /*  22 */ "OpenWrite",
     /*  23 */ "OpenRead",
     /*  24 */ "Transaction",
     /*  25 */ "AutoCommit",
     /*  26 */ "Copy",
     /*  27 */ "Halt",
     /*  28 */ "VRename",
     /*  29 */ "Vacuum",
     /*  30 */ "RowData",
     /*  31 */ "NotExists",
     /*  32 */ "SetCookie",
     /*  33 */ "Move",
     /*  34 */ "Variable",
     /*  35 */ "Pagecount",
     /*  36 */ "VNext",
     /*  37 */ "VDestroy",
     /*  38 */ "TableLock",
     /*  39 */ "RowSetAdd",
     /*  40 */ "LoadAnalysis",
     /*  41 */ "IdxDelete",
     /*  42 */ "Sort",
     /*  43 */ "ResetCount",
     /*  44 */ "Count",
     /*  45 */ "Integer",
     /*  46 */ "Explain",
     /*  47 */ "IncrVacuum",
     /*  48 */ "AggStep",
     /*  49 */ "CreateIndex",
     /*  50 */ "NewRowid",
     /*  51 */ "Return",
     /*  52 */ "Trace",
     /*  53 */ "IfPos",
     /*  54 */ "IdxLT",
     /*  55 */ "Rewind",
     /*  56 */ "SeekGe",
     /*  57 */ "Affinity",
     /*  58 */ "AddImm",
     /*  59 */ "Null",
     /*  60 */ "VColumn",
     /*  61 */ "Clear",
     /*  62 */ "If",
     /*  63 */ "Permutation",
     /*  64 */ "RealAffinity",
     /*  65 */ "HaltIfNull",
     /*  66 */ "Or",
     /*  67 */ "And",
     /*  68 */ "Yield",
     /*  69 */ "AggFinal",
     /*  70 */ "IfZero",
     /*  71 */ "IsNull",
     /*  72 */ "NotNull",
     /*  73 */ "Ne",
     /*  74 */ "Eq",
     /*  75 */ "Gt",
     /*  76 */ "Le",
     /*  77 */ "Lt",
     /*  78 */ "Ge",
     /*  79 */ "Last",
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
     /*  90 */ "Rowid",
     /*  91 */ "Sequence",
     /*  92 */ "NotFound",
     /*  93 */ "BitNot",
     /*  94 */ "String8",
     /*  95 */ "SeekGt",
     /*  96 */ "MakeRecord",
     /*  97 */ "ResultRow",
     /*  98 */ "String",
     /*  99 */ "Goto",
     /* 100 */ "Noop",
     /* 101 */ "VCreate",
     /* 102 */ "RowSetRead",
     /* 103 */ "DropTable",
     /* 104 */ "IdxRowid",
     /* 105 */ "Insert",
     /* 106 */ "Column",
     /* 107 */ "Compare",
     /* 108 */ "VOpen",
     /* 109 */ "CreateTable",
     /* 110 */ "Found",
     /* 111 */ "Seek",
     /* 112 */ "Close",
     /* 113 */ "Savepoint",
     /* 114 */ "Statement",
     /* 115 */ "IfNot",
     /* 116 */ "VBegin",
     /* 117 */ "MemMax",
     /* 118 */ "Next",
     /* 119 */ "Prev",
     /* 120 */ "SeekLe",
     /* 121 */ "MustBeInt",
     /* 122 */ "CollSeq",
     /* 123 */ "Gosub",
     /* 124 */ "ContextPush",
     /* 125 */ "ParseSchema",
     /* 126 */ "Destroy",
     /* 127 */ "IdxGE",
     /* 128 */ "ReadCookie",
     /* 129 */ "Jump",
     /* 130 */ "Real",
     /* 131 */ "Function",
     /* 132 */ "SCopy",
     /* 133 */ "Int64",
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
