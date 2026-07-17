#ifndef DCUCLASSES_H
#define DCUCLASSES_H
//------------------------------------------------------------------------------
// Delphi Versions (referencing dcu32int/DCU32.pas)
#define verD2    2
#define verD3    3
#define verD4    4
#define verD5    5
#define verD6    6
#define verD7    7
#define verD8    8
#define verD2005 9   // 2005         D9->D2005
#define verD2006 10  // 2006, 2007  D10->D2006
#define verD2009 12  // 2009
#define verD2010 14  // 2010
#define verDXE1  15  // XE
#define verDXE2  16  // XE2
#define verDXE3  17  // XE3
#define verDXE4  18  // XE4
#define verDXE5  19  // XE5
#define verDXE6  20  // XE6
#define verDXE7  21  // XE7 & AppMethod
#define verDXE8  22  // XE8
#define verD10   23  // 10 Seattle
#define verD10_1 24  // 10.1 Berlin
#define verD10_2 25  // 10.2 Tokyo
#define verD10_3 26  // 10.3 Rio
#define verD10_4 27  // 10.4 Sydney
#define verD11   28  // 11 Alexandria
#define verD12   29  // 12 Athens
#define verK1    100 // Kylix 1.0
#define verK2    101 // Kylix 2.0
#define verK3    102 // Kylix 3.0

// TDCUPlatform
#define dcuplWin32  0
#define dcuplWin64  1
#define dcuplOsx32  2

// Internal unit types
#define drStop              0
#define drStop_a            0x61    //'a' - Last Tag in all files
#define drStop1             0x63    //'c'
#define drUnit              0x64    //'d'
#define drUnit1             0x65    //'e' - in implementation
#define drImpType           0x66    //'f'
#define drImpVal            0x67    //'g'
#define drDLL               0x68    //'h'
#define drExport            0x69    //'i'
#define drEmbeddedProcStart 0x6A    //'j'
#define drEmbeddedProcEnd   0x6B    //'k'
#define drCBlock            0x6C    //'l'
#define drFixUp             0x6D    //'m'
#define drImpTypeDef        0x6E    //'n' - import of type definition by "A = type B"
#define drSrc               0x70    //'p'
#define drObj               0x71    //'q'
#define drRes               0x72    //'r'
#define drAsm               0x73    //'s' - Found in D5 Debug versions
#define drStop2             0x9F    //'Ÿ'
#define drConst             0x25    //'%'
#define drResStr            0x32    //'2'
#define drType              0x2A    //'*'
#define drTypeP             0x26    //'&'
#define drProc              0x28    //'('
#define drSysProc           0x29    //')'
#define drVoid              0x40    //'@'
#define drVar               0x20    //' '
#define drThreadVar         0x31    //'1'
#define drVarC              0x27    //'''
#define drBoolRangeDef      0x41    //'A'
#define drChRangeDef        0x42    //'B'
#define drEnumDef           0x43    //'C'
#define drRangeDef          0x44    //'D'
#define drPtrDef            0x45    //'E'
#define drClassDef          0x46    //'F'
#define drObjVMTDef         0x47    //'G'
#define drProcTypeDef       0x48    //'H'
#define drFloatDef          0x49    //'I'
#define drSetDef            0x4A    //'J'
#define drShortStrDef       0x4B    //'K'
#define drArrayDef          0x4C    //'L'
#define drRecDef            0x4D    //'M'
#define drObjDef            0x4E    //'N'
#define drFileDef           0x4F    //'O'
#define drTextDef           0x50    //'P'
#define drWCharRangeDef     0x51    //'Q' - WideChar
#define drStringDef         0x52    //'R'
#define drVariantDef        0x53    //'S'
#define drInterfaceDef      0x54    //'T'
#define drWideStrDef        0x55    //'U'
#define drWideRangeDef      0x56    //'V'

// Various tables
#define drCodeLines         0x90
#define drLinNum            0x91
#define drStrucScope        0x92
#define drSymbolRef         0x93
#define drLocVarTbl         0x94
#define drUnitFlags         0x96

// ver70 or higher tags (all of unknown purpose)
#define drUnitAddInfo       0x34    //'4'
#define drInfo98            0x98
#define drConstAddInfo      0x9C
#define drProcAddInfo       0x9E
#define drAssemblyInfo      0x9D    // Ver 2005,2006 .Net

// ver80 or higher tags (all of unknown purpose)
#define drORec              0x6F    //'o' - goes before drCBlock in MSIL
#define drStrConstRec       0x35    //'5'
#define drMetaClassDef      0x57    //'W'

// Kylix-specific flags
#define drUnit4             0x0F    //5-bytes record was observed in QOpenBanner.dcu only

// ver10 and higher tags
#define drSpecVar           0x37    //'7'
#define arClassVarReal      0x2D    //real value
#define arClassVar          0x36    //technical value
#define drCLine             0xA0
#define drA1Info            0xA1
#define drA2Info            0xA2
#define arCopyDecl          0xA3

// ver12 and higher tags
#define drDynArrayDef       0x58    //'X'
#define drTemplateArgDef    0x59    //'Y'
#define drTemplateCall      0x5A    //'Z'
#define drUnicodeStringDef  0x5B    //'['
#define drA5Info            0xA5
#define drA6Info            0xA6
#define drA7Info            0xA7
#define drA8Info            0xA8
#define drDelayedImpInfo    0xB0

// ver13 and higher tags
#define drUnitInlineSrc     0x76    //'v'
#define arAnonymousBlock    0x01

#define arVal               0x21    //'!'
#define arVar               0x22    //'"'
#define arResult            0x23    //'#'
#define arAbsLocVar         0x24    //'$'
#define arLabel             0x2B    //'+'

// XE2 and higher tags
// mode64 only till XE5, all modes since XE6
#define drSegInfo           0xB1
#define drAddrToSegInfo     0xB2

// XE3 and higher tags
#define arFinalFlag         0xC2

// XE4 and higher tags
#define drA9Info            0xA9

// XE7 and higher tags
#define drNextOverload      0xB6

// Delphi 10 (Seattle...Sydney) and higher tags
#define drDependencyInfo    0xB5

// Delphi 11 Alexandria and higher tags
#define drInDcpWin64Info    0xB7

// Delphi 12 Athens and higher tags
#define drDLL1              0xB3

// Fields
#define arFld               0x2C    //','
#define arMethod            0x2D    //'-'
#define arConstr            0x2E    //'.'
#define arDestr             0x2F    //'/'
#define arProperty          0x30    //'0'
#define arSetDeft           0x9A
#define arCDecl             0x81
#define arPascal            0x82
#define arStdCall           0x83
#define arSafeCall          0x84

// Fixup type constants
#define fxAddr      1
#define fxJmpAddr0  2
#define fxDataAddr  3
#define fxJmpAddrXE 5

#define fxStart20   3
#define fxEnd20     4
#define fxStart30   5
#define fxEnd30     6
#define fxStart70   6
#define fxEnd70     7

#define fxVirtMethodMSIL 10 // 0x0A
#define fxStartMSIL      11 // 0x0B
#define fxEndMSIL        12 // 0x0C

#define fxStart100  12
#define fxEnd100    13

#define fxMaxXE     15
#define fxMax       23 // 0x17 Max over all Delphi versions

// XE2 64-bit mode
#define fxAddr64    19
#define fxAddrLo32  23

#define fxStart2010 0
#define fxEnd2010   1

#define FixOfsMask  0xFFFFFF
#define FixOfsShift  24

// TDeclListKind
#define dlMain          0
#define dlMainImpl      1
#define dlArgs          2
#define dlArgsT         3
#define dlEmbedded      4
#define dlFields        5
#define dlClass         6
#define dlInterface     7
#define dlDispInterface 8
#define dlUnitAddInfo   9
#define dlA6            10

// TProcCallKind
#define pcRegister      0
#define pcCdecl         1
#define pcPascal        2
#define pcStdCall       3
#define pcSafeCall      4

// TMethodKind
#define mkProc          0
#define mkMethod        1
#define mkConstructor   2
#define mkDestructor    3

// Local flags
#define lfClass         1    // class procedure
#define lfClassV8up     0x10 // class procedure for Versions 8 up
#define lfPrivate       0
#define lfPublic        2
#define lfProtected     4
#define lfPublished     0xA
#define lfScope         0xE
#define lfDeftProp      0x20
#define lfOverride      0x20
#define lfVirtual       0x40
#define lfDynamic       0x80

// TDeclSecKind
#define skNone          0
#define skLabel         1
#define skConst         2
#define skType          3
#define skVar           4
#define skThreadVar     5
#define skResStr        6
#define skExport        7
#define skProc          8
#define skPrivate       9
#define skProtected     10
#define skPublic        11
#define skPublished     12

#define drAlias         0x5A

// TFloatKind
#define fkReal48        0
#define fkSingle        1
#define fkDouble        2
#define fkExtended      3
#define fkComp          4
#define fkCurrency      5
//------------------------------------------------------------------------------
typedef struct {
    DWord Lo;
    DWord Hi;
} TInt64Rec, *PInt64Rec;

// Pascal ShortString
typedef struct {
    Byte Len;
    char Name[255]; // position of String
} TShortString, *PName;

typedef struct {
    Byte         Tag;
    TShortString Name;
} TNameDef, *PNameDef;

typedef struct {
    PNameDef Def;
    int      Ndx;
} TSrcFileRec, *PSrcFileRec;

typedef struct {
    int   OfsF; // Low 3 bytes - ofs, high 1 byte - B1
    DWord Ndx;
} TFixupRec, *PFixupRec;

typedef struct {
    int       FixCnt;
    PFixupRec Fix;
    Byte     *FixEnd;
} TFixupState, *PFixupState;

typedef struct {
    TFixupState Fx;
    Byte       *CodeBase;
    Byte       *CodeEnd;
    Byte       *CodeStart;
} TFixupMemState, *PFixupMemState;

// for verDXE1 - fix orphaned local types problem
class TTypeDecl;
typedef TTypeDecl *PTypeDecl;
typedef struct {
    PTypeDecl TD;
    int       Depth;
} TEmbeddedTypeInf, *PEmbeddedTypeInf;

//------------------------------------------------------------------------------
class TDCURec;
typedef TDCURec *PDCURec;
typedef void __fastcall (*TTypeUseAction)(PDCURec UseRec, int hDT, DWord *IP);

class TDCURec : public TObject {
public:
    TDCURec();
    virtual PName __fastcall GetName();
    virtual DWord __fastcall SetMem(DWord MOfs, DWord MSz);
    virtual bool __fastcall  NameIsUnique();
    virtual void __fastcall  ShowName(String &OutS);
    virtual void __fastcall  Show(String &OutS);
    virtual void __fastcall  EnumUsedTypes(TTypeUseAction Action, DWord *IP);
    TDCURec                 *Next;
};

class TBaseDef : public TDCURec {
public:
    TBaseDef(PName AName, PNameDef ADef, int AUnit);
    void __fastcall  ShowName(String &OutS);
    void __fastcall  Show(String &OutS);
    void __fastcall  ShowNamed(PName N, String &OutS);
    PName __fastcall GetName();
    DWord __fastcall SetMem(DWord MOfs, DWord MSz);

    PName    FName;
    PNameDef Def;
    int      hUnit;
    int      hDecl;
};

class TImpDef : public TBaseDef {
public:
    TImpDef(Byte AIK, PName AName, int AnInf, PNameDef ADef, int AUnit);
    void __fastcall Show(String &OutS);
    bool __fastcall NameIsUnique();

    char ik;
    bool FNameIsUnique;
    int  Inf;
};

// TUnitImpFlags
#define ufImpl  0
#define ufDLL   1
typedef struct {
    TImpDef  *Ref;
    PName     Name;
    TBaseDef *Decls;
    Byte      Flags;
} TUnitImpRec, *PUnitImpRec;

class TDLLImpRec : public TBaseDef {
public:
    TDLLImpRec(PName AName, int ANdx, PNameDef ADef, int AUnit);
    void __fastcall Show(String &OutS);
    int Ndx;
};

class TImpTypeDefRec : public TImpDef {
public:
    TImpTypeDefRec(PName AName, int AnInf, DWord ARTTISz, PNameDef ADef, int AUnit);
    void __fastcall  Show(String &OutS);
    DWord __fastcall SetMem(DWord MOfs, DWord MSz);

    DWord RTTIOfs, RTTISz;
    int   hImpUnit;
    PName ImpName;
};

class TNameDecl : public TDCURec {
public:
    TNameDecl();
    TNameDecl(bool All);
    ~TNameDecl();
    void __fastcall         ShowName(String &OutS);
    void __fastcall         Show(String &OutS);
    void __fastcall         ShowConstAddInfo(String &OutS);
    virtual void __fastcall ShowDef(bool All, String &OutS);
    PName __fastcall        GetName();
    DWord __fastcall        SetMem(DWord MOfs, DWord MSz);
    virtual Byte __fastcall GetSecKind();
    virtual bool __fastcall IsVisible(Byte LK);
    Byte __fastcall         GetTag();

    PNameDef Def;
    int      hDecl;
    int      ConstAddInfoFlags;
};
typedef TNameDecl *PNameDecl;

class TNameFDecl : public TNameDecl {
public:
    TNameFDecl(bool NoInf);
    void __fastcall Show(String &OutS) override;
    bool __fastcall IsVisible(Byte LK) override;

    int F, F1;
    int Inf;
    int B2; // D8+
};

class TTypeDecl : public TNameFDecl {
public:
    TTypeDecl();
    bool __fastcall  IsVisible(Byte LK);
    void __fastcall  Show(String &OutS);
    void __fastcall  EnumUsedTypes(TTypeUseAction Action, DWord *IP);
    PName __fastcall GetName();
    DWord __fastcall SetMem(DWord MOfs, DWord MSz);
    Byte __fastcall  GetSecKind();

    int hDef;
};

class TVarDecl : public TNameFDecl {
public:
    TVarDecl();
    void __fastcall Show(String &OutS);
    void __fastcall EnumUsedTypes(TTypeUseAction Action, DWord *IP);
    Byte __fastcall GetSecKind();

    int   hDT;
    DWord Ofs;
};

class TVarVDecl : public TVarDecl {
    // In DXE2 win64 an auxiliary variable __puiHead has memory image
    TVarVDecl();
    void __fastcall  Show(String &OutS);
    DWord __fastcall SetMem(DWord MOfs, DWord MSz);

    DWord Sz;
};

class TVarCDecl : public TVarDecl {
public:
    TVarCDecl(bool OfsValid);
    void __fastcall  Show(String &OutS);
    DWord __fastcall SetMem(DWord MOfs, DWord MSz);
    Byte __fastcall  GetSecKind();

    DWord Sz;
    DWord OfsR;
};

class TAbsVarDecl : public TVarDecl {
public:
    TAbsVarDecl();
    void __fastcall Show(String &OutS);
};

class TTypePDecl : public TVarCDecl {
public:
    TTypePDecl();
    void __fastcall Show(String &OutS);
    bool __fastcall IsVisible(Byte LK);
};

class TThreadVarDecl : public TVarDecl {
public:
    TThreadVarDecl();
    Byte __fastcall GetSecKind();
};

// In Delphi>=8 they started to create this kind of records for string constants
// and other data blocks (instead of TProcDecl, which was used earlier)
class TStrConstDecl : public TNameFDecl {
public:
    TStrConstDecl();
    DWord __fastcall SetMem(DWord MOfs, DWord MSz);
    Byte __fastcall  GetSecKind();
    void __fastcall  MemRefFound();
    // bool __fastcall IsVisible(Byte LK);
    void __fastcall Show(String &OutS);
    void __fastcall EnumUsedTypes(TTypeUseAction Action, DWord *IP);

    int   hDT;
    DWord Ofs;
    DWord Sz;
    // DWord   FX;
    // DWord   FX1;
    // bool    FMemUsed;
};

class TLabelDecl : public TNameDecl {
public:
    TLabelDecl();
    void __fastcall Show(String &OutS);
    Byte __fastcall GetSecKind();
    bool __fastcall IsVisible(Byte LK);

    DWord Ofs;
};

class TExportDecl : public TNameDecl {
public:
    TExportDecl();
    void __fastcall Show(String &OutS);
    Byte __fastcall GetSecKind();

    int hSym, Index;
};

class TLocalDecl : public TNameDecl {
public:
    TLocalDecl(Byte LK);
    // void __fastcall ShowName(String& OutS);
    void __fastcall Show(String &OutS);
    void __fastcall EnumUsedTypes(TTypeUseAction Action, DWord *IP);
    Byte __fastcall GetLocFlagsSecKind();
    Byte __fastcall GetSecKind();
    // bool __fastcall IsVisible(Byte LK);

    int LocFlags;
    int LocFlagsX; // Ver>=8 private, protected, public, published
    int hDT;
    int NdxB;
    int Ndx;
};

class TMethodDecl : public TLocalDecl {
public:
    TMethodDecl(Byte LK);
    void __fastcall Show(String &OutS);

    bool InIntrf;
    int  hImport;
};

class TClassVarDecl : public TLocalDecl {
public:
    TClassVarDecl(Byte LK);
    void __fastcall Show(String &OutS);
    Byte __fastcall GetSecKind();
};

class TPropDecl : public TNameDecl {
public:
    TPropDecl();
    String __fastcall PutOp(String Name, int hOp);
    void __fastcall   Show(String &OutS);
    void __fastcall   EnumUsedTypes(TTypeUseAction Action, DWord *IP);
    Byte __fastcall   GetSecKind();

    int LocFlags;
    int LocFlagsX; // Ver>=8 private, protected, public, published
    int hDT;
    int Ndx;
    int hIndex;
    int hRead;
    int hWrite;
    int hStored;
    int hDeft;
};

class TDispPropDecl : public TLocalDecl {
public:
    TDispPropDecl(Byte LK);
    void __fastcall Show(String &OutS);
};

class TConstDeclBase : public TNameFDecl {
public:
    TConstDeclBase();
    void __fastcall ReadConstVal();
    void __fastcall ShowValue(String &OutS);
    void __fastcall Show(String &OutS);
    void __fastcall EnumUsedTypes(TTypeUseAction Action, DWord *IP);
    Byte __fastcall GetSecKind();

    int     hDT;
    DWord   Kind; // hX Ver>4
    // 0 - scalar, 1 - string (offset=8), 2 - resourcestring,
    // 3-float, 4 - set,
    //[ver>=verD12] 5 - Unicode string (offset=12)
    Byte    *ValPtr;
    DWord   ValSz;
    int     Val;
};

class TConstDecl : public TConstDeclBase {
public:
    TConstDecl();
    bool __fastcall IsVisible(Byte LK);
};

class TResStrDef : public TVarCDecl {
public:
    TResStrDef();
    void __fastcall Show(String &OutS);
    Byte __fastcall GetSecKind();

    DWord OfsR;
};

class TSetDeftInfo : public TNameDecl {
public:
    TSetDeftInfo();
    void __fastcall Show(String &OutS);

    int hConst, hArg;
};

class TCopyDecl : public TNameDecl {
public:
    TCopyDecl();
    void __fastcall Show(String &OutS);
    Byte __fastcall GetSecKind();

    int hBase;
    TNameDecl *Base; // Just in case and for convenience
};

typedef struct {
    int sym;   // Symbol # in the symbol table, 0 - proc data end
    int ofs;   // Offset in procedure code
    int frame; //-1(0x7f)-symbol end, else - symbol start 0-EAX, 1-EDX, 2-ECX, 3-EBX, 4-ESI...
} TLocVarRec, *PLocVarRec;

typedef struct {
    int  hDecl;
    int  Ofs;
    bool IsVar;
    bool InReg;
} TRegDebugInfo, *PRegDebugInfo;

// ProcDeclInfo
typedef struct {
    int    ID;
    DWord  Offset;
    DWord  Size;
    String Name;
    Word   ModuleID;
    bool   Embedded;   // if true, contains embedded procs
    char   DumpType;   //'C' - code, 'D' - data
    Byte   MethodKind; //'M'-method,'P'-procedure,'F'-function,'C'-constructor,'D'-destructor
    Byte   CallKind;
    int    VProc;
    DWord  DumpSz;  // Size of binary data
    DWord  DumpOfs; // Offset of binary data
    String TypeDef;
    TList *Args;
    TList *Locals;
    TList *Fixups;
} PROCDECLINFO, *PPROCDECLINFO;

class TProcDecl : public TNameFDecl {
public:
    TProcDecl(TNameDecl *AnEmbedded, bool NoInf);
    ~TProcDecl();
    bool __fastcall  IsUnnamed();
    DWord __fastcall SetMem(DWord MOfs, DWord MSz);
    Byte __fastcall  GetSecKind();
    void __fastcall  ShowArgs(String &OutS, PPROCDECLINFO pInfo);
    bool __fastcall  IsProc();
    void __fastcall  ShowDef(bool All, String &OutS);
    void __fastcall  Show(String &OutS);
    void __fastcall  EnumUsedTypes(TTypeUseAction Action, DWord *IP);
    bool __fastcall  IsVisible(Byte LK);
    // String __fastcall GetRegDebugInfo(int ProcOfs, int hReg, int Ofs, int* hDecl);

    DWord      CodeOfs, AddrBase;
    DWord      Sz;
    int        B0;
    int        VProc;
    int        hDTRes;
    int        hClass;
    TNameDecl *Args;
    TNameDecl *Locals;
    TNameDecl *Embedded;
    Byte       CallKind;
    Byte       MethodKind; // maybe this information is encoded by some flag, but
    // I can't detect it. Maybe it would be enough to analyse the structure of
    // the procedure name, but this way it will be safer.
    bool       JustData; // This flag is turned on by Fixups from String-typed consts
    PLocVarRec FProcLocVarTbl;
    int        FProcLocVarCnt;
    TDCURec   *FTemplateArgs;
};
typedef TProcDecl *PProcDecl;

class TSysProcDecl : public TNameDecl {
public:
    TSysProcDecl();
    void __fastcall Show(String &OutS);
    Byte __fastcall GetSecKind();

    int F;
    int Ndx;
};

// Starting from Delphi 8, Borland begins to give complete proc. defs to system procedures
class TSysProc8Decl : public TProcDecl {
public:
    TSysProc8Decl();

    int F;
    int Ndx;
};

// Ver 7.0 and higher, MSIL
class TUnitAddInfo : public TNameFDecl {
public:
    TUnitAddInfo();
    ~TUnitAddInfo();
    bool __fastcall IsVisible(Byte LK);

    // FVer 8.0 and higher, MSIL
    int B;
    TNameDecl *Sub;
};

class TSpecVar : public TVarDecl {
public:
    TSpecVar();
    void __fastcall Show(String &OutS);
};

class TTypeDef : public TBaseDef {
public:
    TTypeDef();
    ~TTypeDef();
    void __fastcall           ShowBase();
    virtual int __fastcall    ShowValue(Byte *DP, DWord DS, String &OutS);
    void __fastcall           Show(String &OutS);
    DWord __fastcall          SetMem(DWord MOfs, DWord MSz);
    virtual String __fastcall GetOfsQualifier(int Ofs);
    virtual String __fastcall GetRefOfsQualifier(int Ofs);

    int   RTTISz; // Size of RTTI for type, if available
    int   Sz;     // Size of corresponding variable
    int   hAddrDef;
    int   X;
    DWord RTTIOfs;
};

class TRangeBaseDef : public TTypeDef {
public:
    TRangeBaseDef();
    void __fastcall GetRange(PInt64Rec Lo, PInt64Rec Hi);
    int __fastcall  ShowValue(Byte *DP, DWord DS, String &OutS);
    void __fastcall Show(String &OutS);
    void __fastcall EnumUsedTypes(TTypeUseAction Action, DWord *IP);

    int   hDTBase;
    Byte *LH;
    Byte  B;
};

class TRangeDef : public TRangeBaseDef {
public:
    TRangeDef();
};

class TEnumDef : public TRangeBaseDef {
public:
    TEnumDef();
    ~TEnumDef();
    int __fastcall  ShowValue(Byte *DP, DWord DS, String &OutS);
    void __fastcall Show(String &OutS);

    int    Ndx;
    TList *NameTbl;
};

class TFloatDef : public TTypeDef {
public:
    TFloatDef();
    String __fastcall GetKindName();
    int __fastcall    ShowValue(Byte *DP, DWord DS, String &OutS);
    void __fastcall   Show(String &OutS);

    Byte Kind;
};

class TPtrDef : public TTypeDef {
public:
    TPtrDef();
    bool __fastcall   ShowRefValue(int Ndx, DWord Ofs, String &OutS);
    int __fastcall    ShowValue(Byte *DP, DWord DS, String &OutS);
    void __fastcall   Show(String &OutS);
    void __fastcall   EnumUsedTypes(TTypeUseAction Action, DWord *IP);
    String __fastcall GetRefOfsQualifier(int Ofs);

    int hRefDT;
};

class TTextDef : public TTypeDef {
public:
    TTextDef();
    void __fastcall Show(String &OutS);
};

class TFileDef : public TTypeDef {
public:
    TFileDef();
    void __fastcall Show(String &OutS);
    void __fastcall EnumUsedTypes(TTypeUseAction Action, DWord *IP);

    int hBaseDT;
};

class TSetDef : public TTypeDef {
public:
    TSetDef();
    int __fastcall  ShowValue(Byte *DP, DWord DS, String &OutS);
    void __fastcall Show(String &OutS);
    void __fastcall EnumUsedTypes(TTypeUseAction Action, DWord *IP);

    Byte BStart; // 0-based start byte number
    int  hBaseDT;
};

// This type is required to make it parent of TStringDef
class TArrayDef0 : public TTypeDef {
public:
    TArrayDef0(bool IsStr);
    int __fastcall  ShowValue(Byte *DP, DWord DS, String &OutS);
    void __fastcall Show(String &OutS);
    void __fastcall EnumUsedTypes(TTypeUseAction Action, DWord *IP);

    Byte B1;
    int  hDTNdx;
    int  hDTEl;
};

class TArrayDef : public TArrayDef0 {
public:
    TArrayDef(bool IsStr);
    String __fastcall GetOfsQualifier(int Ofs);
};

class TShortStrDef : public TArrayDef {
public:
    TShortStrDef();
    int __fastcall  ShowValue(Byte *DP, DWord DS, String &OutS);
    void __fastcall Show(String &OutS);

    int CP; // for Ver>=VerD12
};

class TStringDef : public TArrayDef {
public:
    TStringDef();
    bool __fastcall   ShowRefValue(int Ndx, DWord Ofs, String &OutS);
    int __fastcall    ShowValue(Byte *DP, DWord DS, String &OutS);
    void __fastcall   Show(String &OutS);
    String __fastcall GetRefOfsQualifier(int Ofs);

    int CP; // for Ver>=VerD12
};

class TVariantDef : public TTypeDef {
public:
    TVariantDef();
    void __fastcall Show(String &OutS);

    Byte B;
};

class TObjVMTDef : public TTypeDef {
public:
    TObjVMTDef();
    void __fastcall Show(String &OutS);

    int hObjDT;
    int Ndx1;
};

class TRecBaseDef : public TTypeDef {
public:
    TRecBaseDef();
    ~TRecBaseDef();
    void __fastcall        ReadFields(Byte LK);
    int __fastcall         ShowFieldValues(Byte *DP, DWord DS, String &OutS);
    void __fastcall        EnumUsedTypes(TTypeUseAction Action, DWord *IP);
    virtual int __fastcall GetParentType();
    TPropDecl *__fastcall  GetFldProperty(PNameDecl Fld, int hDT);
    String __fastcall      GetFldOfsQualifier(int Ofs, int TotSize, bool Sorted);

    TNameDecl *Fields;
};

class TRecDef : public TRecBaseDef {
public:
    TRecDef();
    int __fastcall    ShowValue(Byte *DP, DWord DS, String &OutS);
    void __fastcall   Show(String &OutS);
    String __fastcall GetOfsQualifier(int Ofs);

    Byte B2;
};

class TProcTypeDef : public TRecBaseDef {
public:
    TProcTypeDef();
    int __fastcall    ShowValue(Byte *DP, DWord DS, String &OutS);
    bool __fastcall   IsProc();
    String __fastcall ProcStr();
    void __fastcall   ShowDecl(char *Braces, String &OutS);
    void __fastcall   Show(String &OutS);
    void __fastcall   EnumUsedTypes(TTypeUseAction Action, DWord *IP);

    int     Ndx0; // B0: Byte; //Ver>2
    int     hDTRes;
    Byte   *AddStart;
    DWord   AddSz; // FVer>2
    Byte    CallKind;
    PDCURec AddInfo; // for Ver>=verD2009
};

class TObjDef : public TRecBaseDef {
public:
    TObjDef();
    int __fastcall    ShowValue(Byte *DP, DWord DS, String &OutS);
    void __fastcall   Show(String &OutS);
    void __fastcall   EnumUsedTypes(TTypeUseAction Action, DWord *IP);
    int __fastcall    GetParentType();
    String __fastcall GetOfsQualifier(int Ofs);

    Byte B03;
    int  hParent;
    Byte BFE;
    int  Ndx1;
    Byte B00;
};

class TClassDef : public TRecBaseDef {
public:
    TClassDef();
    ~TClassDef();
    int __fastcall          ShowValue(Byte *DP, DWord DS, String &OutS);
    void __fastcall         Show(String &OutS);
    int __fastcall          GetParentType();
    String __fastcall       GetRefOfsQualifier(int Ofs);
    virtual void __fastcall ReadBeforeIntf();

    int  hParent;
    int  InstBaseRTTISz; // Size of RTTI for the type, if available
    int  InstBaseSz;     // Size of corresponding variable
    int  InstBaseV;
    int  VMCnt;  // number of virtual methods
    int  NdxFE;  // BFE: Byte
    int  Ndx00a; // B00a: Byte
    int  B04;
    int  ICnt;
    int *ITbl;
};

class TMetaClassDef : public TClassDef {
public:
    TMetaClassDef();
    void __fastcall ReadBeforeIntf();

    int hCl;
};

class TInterfaceDef : public TRecBaseDef {
public:
    TInterfaceDef();
    void __fastcall Show(String &OutS);

    int   hParent;
    int   VMCnt;
    PGUID GUID;
    Byte  B;
};

class TVoidDef : public TTypeDef {
public:
    TVoidDef();
    void __fastcall Show(String &OutS);
};

class TA6Def : public TDCURec {
public:
    TA6Def();
    ~TA6Def();
    void __fastcall Show(String &OutS);

    PNameDecl Args;
};

class TA7Def : public TDCURec {
public:
    TA7Def();
    ~TA7Def();
    void __fastcall Show(String &OutS);

    int  hClass;
    int  Cnt;
    int *Tbl;
};

class TDelayedImpRec : public TNameDecl {
public:
    TDelayedImpRec();
    void __fastcall Show(String &OutS);

    int Inf;
    int F;
};

class TORecDecl : public TNameDecl {
public:
    TORecDecl();
    ~TORecDecl();
    void __fastcall Show(String &OutS);

    int       DW;
    Byte      B0;
    Byte      B1;
    PNameDecl Args;
};

// for Ver>=VerD12
class TDynArrayDef : public TPtrDef {
public:
    TDynArrayDef();
    void __fastcall   Show(String &OutS);
    String __fastcall GetRefOfsQualifier(int Ofs);
};

// for Ver>=VerD12 - template support
class TTemplateArgDef : public TTypeDef {
public:
    TTemplateArgDef();
    ~TTemplateArgDef();
    void __fastcall Show(String &OutS);

    int  Cnt, V5;
    int *Tbl;
};

// for Ver>=VerD12 - template support
class TTemplateCall : public TTypeDef {
public:
    TTemplateCall();
    ~TTemplateCall();
    void __fastcall Show(String &OutS);
    void __fastcall EnumUsedTypes(TTypeUseAction Action, DWord *IP);

    int  hDT;
    int  Cnt;
    int *Args;
    int  hDTFull;
};

//------------------------------------------------------------------------------
#endif
