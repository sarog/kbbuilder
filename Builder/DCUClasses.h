#ifndef DCUCLASSES_H
#define DCUCLASSES_H
#include <algorithm>
//------------------------------------------------------------------------------

// todo: new:
enum class TDelphiVersion : int {
    D2    = 2,
    D3    = 3,
    D4    = 4,
    D5    = 5,
    D6    = 6,
    D7    = 7,
    D8    = 8,
    D2005 = 9,
    D2006 = 10,
    D2009 = 12,
    D2010 = 14,
    DXE1  = 15,
    DXE2  = 16,
    DXE3  = 17,
    DXE4  = 18,
    DXE5  = 19,
    DXE6  = 20,
    DXE7  = 21,  // XE7 & AppMethod
    DXE8  = 22,
    D10   = 23,  // 10 Seattle
    D10_1 = 24,  // 10.1 Berlin
    D10_2 = 25,  // 10.2 Tokyo
    D10_3 = 26,  // 10.3 Rio
    D10_4 = 27,  // 10.4 Sydney
    D11   = 28,  // 11 Alexandria
    D12   = 29,  // 12 Athens
    D13   = 29,  // 13 Florence
    K1    = 100, // Kylix 1
    K2    = 101, // Kylix 2
    K3    = 102  // Kylix 3
};

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
#define verD13   29  // 13 Florence
#define verK1    100 // Kylix 1.0
#define verK2    101 // Kylix 2.0
#define verK3    102 // Kylix 3.0

const static int MaxDelphiVer = 30;

// TDCUPlatform
#define dcuplWin32       0
#define dcuplWin64       1
#define dcuplOsx32       2
#define dcuplOsx64       3
#define dcuplOsxArm64    4
#define dcuplIOSEmulator 5
#define dcuplIOSSimArm64 6
#define dcuplIOSDevice   7
#define dcuplIOSDevice64 8
#define dcuplAndroid     9
#define dcuplAndroid64   10
#define dcuplLinux64     11

enum class TDCUPlatform : int {
    Win32       = 0,
    Win64       = 1,
    Osx32       = 2,
    Osx64       = 3,
    OsxArm64    = 4,
    IOSEmulator = 5,
    IOSSimArm64 = 6,
    IOSDevice   = 7,
    IOSDevice64 = 8,
    Android     = 9,
    Android64   = 10,
    Linux64     = 11
};

// Internal unit types
#define drStop              0x0
#define drStop_a            0x61    //'a' - Last Tag in all files
#define drAssemblyData      0x62    //'b' - (Packaged && MSIL) The data structure was found in .<PackageName> units of D8 packages
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
#define drAssemblySrc       0x74    //'t' (Packaged && MSIL)
#define drStop2             0x9F    //'Ÿ'
#define drConst             0x25    //'%'
#define drResStr            0x32    //'2'
#define drType              0x2A    //'*'
#define drTypeP             0x26    //'&'
#define drProc              0x28    //'('
#define drSysProc           0x29    //')'
#define drVar               0x20    //' '
#define drVarC              0x27    //'''
#define drThreadVar         0x31    //'1'
#define drVoid              0x40    //'@'
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
#define drUnitAddInfo       0x34    //'4' if Ver < 10
// #define drUnitAddInfo       0x35    //'5' if Ver >= 10
#define drCPPFlags          0x98    // was drInfo98
#define drConstAddInfo      0x9C    // D7 - caused by the "platform" keyword

#define drProcAddInfo       0x9E
#define drAssemblyInfo      0x9D    // (2005/2006 .NET) The unit is from package generated for assemmbly

// ver80 or higher tags (all of unknown purpose)
#define drORec              0x6F    //'o' - goes before drCBlock in MSIL
#define drStrConstRec       0x35    //'5'
#define drMetaClassDef      0x57    //'W'

// Kylix-specific flags
#define drUnit4             0x0F    // 5-bytes record was observed in QOpenBanner.dcu only

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
#define drDLLInfo1              0xB3    // iosSimArm64, iosDevice64

// Fields
#define arFld               0x2C    //','
#define arMethod            0x2D    //'-'
#define arConstr            0x2E    //'.'
#define arDestr             0x2F    //'/'
#define arProperty          0x30    //'0'
#define arSetDeft           0x9A    // Set Default parameter value
#define arCDecl             0x81
#define arPascal            0x82
#define arStdCall           0x83
#define arSafeCall          0x84

// Fixup type constants
#define fxAddr      1 // absolute data address should be substituted
#define fxJmpAddr0  2 // relative data address should be substituted (for jmp or call near instructions).
#define fxDataAddr  3 // imported data reference (from another unit) this interpretation appears only in Delphi 3.0+
#define fxJmpAddrXE 5

#define fxStart20   3 // Start of block marker in Delhi 2.0 units
#define fxEnd20     4 // End of block marker in Delhi 2.0 units
#define fxStart30   5 // Start of block marker in Delhi 3.0+ units
#define fxEnd30     6 // End of block marker in Delhi 3.0-6.0 and Kylix units
#define fxStart70   6 // Start of block marker in Delhi 7.0, 2005 (WIN32) units
#define fxEnd70     7 // End of block marker in Delhi 7.0, 2005 (WIN32) units

#define fxVirtMethodMSIL 10 // 0x0A
#define fxStartMSIL      11 // 0x0B Start of block marker in Delhi 8.0, 2005 .net units
#define fxEndMSIL        12 // 0x0C End of block marker in Delhi 8.0, 2005 .net units

#define fxStart100  12 // 0x0C Start of block marker in Delhi 2006 (WIN32) units
#define fxEnd100    13 // 0x0D End of block marker in Delhi 2006 (WIN32) units

#define fxMaxXE     15
#define fxMax       23 // 0x17 Max over all Delphi versions

// XE2 64-bit mode
#define fxAddr64    19
#define fxAddrLo32  23

#define fxStart2010 0
#define fxEnd2010   1

#define FixOfsMask  0xFFFFFF // used to extract the fixup Offset value from the OfsF field.
#define FixOfsShift  24

// todo:
enum class TDeclListKind : int {
    Main          = 0,
    MainImpl      = 1,
    Args          = 2,
    ArgsT         = 3,
    Embedded      = 4,
    Fields        = 5,
    Class         = 6,
    Interface     = 7,
    DispInterface = 8,
    UnitAddInfo   = 9,
    A6            = 10
};

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

enum class TProcCallKind : int {
    Register      = 0,
    Cdecl         = 1,
    Pascal        = 2,
    StdCall       = 3,
    SafeCall      = 4
};

// TProcCallKind
#define pcRegister      0
#define pcCdecl         1
#define pcPascal        2
#define pcStdCall       3
#define pcSafeCall      4

enum class TMethodKind : int {
    Proc          = 0,
    Method        = 1,
    Constructor   = 2,
    Destructor    = 3
};

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
#define lfStrict        0x10
// lfStrictPrivate = lfStrict or lfPrivate;
// lfStrictProtected = lfStrict or lfProtected;
#define lfRegister      0x8 // stored in register in procedure code
#define lfPublished     0xA
#define lfScope         0xE
#define lfParmFlagsMask 0x30
#define lfStackParm     0x10 // parameter is on stack?
#define lfRegisterParm  0x00 // parameter is in register (optimization on)
#define lfRegister1Parm 0x30 // parameter is in register (optimization off)
#define lfDeftProp      0x20
#define lfOverride      0x20
#define lfVirtual       0x40
#define lfDynamic       0x80

enum class TDeclSecKind : int {
    None = 0,
    Label = 1,
    Const = 2,
    Type = 3,
    Var = 4,
    ThreadVar = 5,
    ResStr = 6,
    Export = 7,
    Proc = 8,
    Private = 9,
    Protected = 10,
    Public = 11,
    Published = 12
};

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
// skStrictPrivate
// skStrictProtected

#define cvScalar         0
#define cvString         1
#define cvResourceString 2
#define cvFloat          3
#define cvSet            4
#define cvUnicodeString  5
#define cvxPointer       MAXINT // Aux const for inline code, not in DCU

#define drAlias         0x5A

enum class TFloatKind : byte { Real48 = 0, Single = 1, Double = 2, Extended = 3, Comp = 4, Currency = 5 };

// TFloatKind
#define fkReal48        0
#define fkSingle        1
#define fkDouble        2
#define fkExtended      3
#define fkComp          4
#define fkCurrency      5

#define fkExtra 0x80

// enum class TShowProcCtx { spcMain, spcMainImpl, spcOther };
// TShowProcCtx
#define spcMain     0
#define spcMainImpl 1
#define spcOther    2
//------------------------------------------------------------------------------
using TNDX       = int;
using TDefNDX    = TNDX;
using TNDXTbl    = TNDX[256];
using PNDXTbl    = TNDXTbl *;
using PPNDXTbl   = PNDXTbl *;
using TDCURecTag = Byte;

using PDef = Byte **;

// Pascal ShortString // PShortString, used by PShortName
// Since 1.15.1: PName was changed from PShortString to the data structure TNameRec
//               describing long strings (longer than 255 bytes), which can happen as
//               a result of mangling or template instantiation starting from D2009.
// typedef struct {
//     Byte Len;
//     char Name[255];
// } TShortString; // , *PName;

#pragma pack(push, 1)
union TNameRecData {
    ShortString S;
    struct {
        std::uint8_t bLen;      // std::uint8_t // Byte
        std::int32_t dwLen;     // std::int32_t // LongInt
        AnsiChar     lS[65536]; // array[Word] of AnsiChar
    };
    TNameRecData();
    ~TNameRecData() { S.~ShortString(); }
    TNameRecData(const TNameRecData& src) {
        std::memcpy(this, &src, sizeof(TNameRecData));
    }
    TNameRecData& operator=(const TNameRecData& src) {
        if (this != &src) {
            std::memcpy(this, &src, sizeof(TNameRecData));
        }
        return *this;
    }
};
#pragma pack(pop)

// using PNameRecData = TNameRecData*;

struct TAnsiStrRec {
    char *CP;
    DWord Len;
};

struct TNameRec;
using PName = TNameRec*;

struct TNameRec {
public:
    TNameRecData D;
    void __fastcall GetStrInfo(TAnsiStrRec &SR);

public:
    TNameRec() = default;
    TNameRec(const TNameRec& src) : D(src.D) {}
    TNameRec& operator=(const TNameRec& src) {
        if (this != &src) {
            D = src.D;
        }
        return *this;
    }

    bool __fastcall       IsEmpty();
    AnsiChar __fastcall   Get1stChar();
    AnsiString __fastcall GetStr();
    AnsiString __fastcall GetRightStr(std::int32_t dl);
    bool __fastcall       Eq(PName N);
    bool __fastcall       EqS(const ShortString &S);
    bool __fastcall       HasChar(AnsiChar ch);
    bool __fastcall       IsAuxName();
    LongInt __fastcall    GetHash();             // LongInt
    LongInt __fastcall    GetRightHash(int Ofs); // LongInt
};

using PShortName = PShortString;

typedef struct {
    DWord Lo;
    DWord Hi;
} TInt64Rec, *PInt64Rec;

using PNameRec = TNameRec*;

// DCU_IN.pas
typedef struct {
    TDCURecTag  Tag; // TDCURecTag
    // was: TShortString Name;
    // new:
    TNameRec Name;
} TNameDef, *PNameDef;

typedef struct {
    PNameDef    Def;
    int         Ndx;
    int         FT; // File Time (LongInt)
} TSrcFileRec, *PSrcFileRec;

typedef struct {
    int  OfsF; // Low 3 bytes - ofs, high 1 byte - B1
    TNDX Ndx;  // The index of the target address
} TFixupRec, *PFixupRec;

typedef struct {
    TFixupRec* items;  // Array of TFixupRec (indexed by Word/unsigned short)
    unsigned int count;
} TFixupTbl, *PFixupTbl;

typedef struct {
    int       FixCnt;
    PFixupRec Fix;
    Byte     *FixEnd;
    // Byte     *FixUnit; // TUnit
} TFixupState, *PFixupState;

typedef struct {
    TFixupState Fx;
    Byte       *CodeBase;
    Byte       *CodeEnd;
    Byte       *CodeStart;
} TFixupMemState, *PFixupMemState;

// for verDXE1 - fix orphaned local types problem
class TTypeDecl;
using PTypeDecl = TTypeDecl *;
typedef struct {
    PTypeDecl TD;
    int       Depth;
} TEmbeddedTypeInf, *PEmbeddedTypeInf;

// typedef Byte TSegKind;
enum class TSegKind {
    None  = 0,
    Text  = 1,
    IText = 2,
    IData = 3,
    BSS   = 4,
    TLS   = 5,
    PData = 6,
    XData = 7,
    TBSS  = 8,
    RDATA = 9
};

// TSegKind
/*
#define tskNone 0
#define tskText 1 // +OsX
#define tskItext 2
#define tskiData 3 // +OsX
#define tskBss 4 // +OsX
#define tskTLS 5
#define tskPData 6
#define tskXdata 7
#define tskTbss 8 // +OsX
#define tskRData 9 // +OsX
*/

using TSegKindTbl = TSegKind[256];
using PSegKindTbl = TSegKindTbl *;

using TDCUFileTime = int;
using TIncPtr = Byte*; // PAnsiChar
// PtrInt = {$IFDEF CPUX64}NativeInt{$ELSE}Integer{$ENDIF};

struct TScanState {
    TIncPtr StartPos;
    TIncPtr CurPos;
    TIncPtr EndPos;
};

//------------------------------------------------------------------------------
class TDCURec;
using PDCURec = TDCURec*; // PTDCURec

typedef void __fastcall (*TTypeUseAction)(PDCURec UseRec, TDefNDX hDT, DWord *IP);

class TDCURecVisitor; // Pattern "Visitor" for TDCURec class hierarchy

class TDCURec : public TObject {
protected:
    TDCURec *FNext;
    virtual PName __fastcall GetName();
public:
    TDCURec();
    virtual DWord __fastcall      SetMem(DWord MOfs, DWord MSz);
    virtual bool __fastcall       NameIsUnique();
    virtual void __fastcall       Visit(TDCURecVisitor *Visitor);
    virtual void __fastcall       ShowName(String &OutS);
    virtual void __fastcall       Show(String &OutS);
    virtual void __fastcall       EnumUsedTypes(TTypeUseAction Action, DWord *IP);
    virtual TDCURecTag __fastcall GetTag(); // Byte
    virtual bool __fastcall       IsVisible(Byte LK);
    virtual Byte __fastcall       GetSecKind(); // TDeclSecKind
    virtual void __fastcall       ShowDef(bool All, String &OutS);
    virtual void __fastcall       ListAppend(TDCURec *List);
    virtual void __fastcall       SetSegKind(TSegKind V);

    // These are overridden separately
    // for verD_XE - fix orphaned local types problem
    // virtual TDeclSecKind __fastcall GetSecKind(); // Byte

    // TDCURec *Next; // FNext
    __property PName Name = { read = GetName };
    __property TDCURec *Next = { read = FNext, write = FNext };
};

class TBaseDef : public TDCURec {
protected:
    PName __fastcall GetName() override;

public:
    TBaseDef(PName AName, PDef ADef, int AUnit);
    void __fastcall  Visit(TDCURecVisitor *Visitor) override;
    void __fastcall  ShowName(String &OutS) override;
    void __fastcall  Show(String &OutS) override;
    void __fastcall  ShowNamed(PName N, String &OutS);
    DWord __fastcall SetMem(DWord MOfs, DWord MSz) override;

    PName    FName;
    PDef     Def;
    int      hUnit;
    int      hDecl;
};

using TImpKind = AnsiChar; // char

class TImpDef : public TBaseDef {
public:
    TImpDef(TImpKind AIK, PName AName, int AnInf, PDef ADef, int AUnit);
    void __fastcall Visit(TDCURecVisitor *Visitor) override;
    void __fastcall Show(String &OutS) override;
    bool __fastcall NameIsUnique() override;

    TImpKind ik; // char
    bool FNameIsUnique;
    int  Inf;
};

class TUnitImpDef : public TImpDef {
public:
    TUnitImpDef(TImpKind AIK, PName AName, int AnInf, PDef ADef, int AUnit);
    void __fastcall Visit(TDCURecVisitor *Visitor) override;
    void __fastcall Show(String &OutS) override;

    AnsiString sPackage; // for .NET // was String
};

enum class TUnitImpFlags : Byte { None = 0, Impl = 1, DLL = 2, DLL1 = 3};
// TUnitImpFlags
/*#define ufImpl 0
#define ufDLL 1
#define ufDLL1 2*/
typedef struct {
    TUnitImpDef  *Ref;
    PName         Name;
    TBaseDef     *Decls;
    TUnitImpFlags Flags; // TUnitImpFlags
    // TUnit       U;
} TUnitImpRec, *PUnitImpRec;

class TDLLImpRec : public TBaseDef {
public:
    TDLLImpRec(PName AName, int ANdx, PDef ADef, int AUnit);
    void __fastcall Visit(TDCURecVisitor *Visitor) override;
    void __fastcall Show(String &OutS) override;
    int Ndx;
};

class TImpTypeDefRec : public TImpDef {
public:
    TImpTypeDefRec(PName AName, int AnInf, DWord ARTTISz, PDef ADef, int AUnit);
    void __fastcall  Visit(TDCURecVisitor *Visitor) override;
    void __fastcall  Show(String &OutS) override;
    DWord __fastcall SetMem(DWord MOfs, DWord MSz) override;

    DWord RTTIOfs, RTTISz;
    int   hImpUnit;
    PName ImpName;
};

struct TConstValInfoBase {
    DWord Kind; // Ver>4 // Cardinal
                // O - scalar, 1 - string (offset=8), 2 - resourcestring,
                // 3-float, 4 - set,
                // [ver>=verD12] 5 - Unicode string (offset=12)
    Byte *ValPtr;
    DWord ValSz; // Cardinal
    int Val;
    void __fastcall Show0(TDefNDX hDT, bool IsNamed);
};

struct TConstValInfo : TConstValInfoBase {
    TDefNDX hDT;
    void __fastcall Read();
    void __fastcall Show(bool IsNamed);
};

// Name Declaration Modifiers - store some important information from
// drConstAddInfo records and other records like this if any
class TDeclModifier;
using TDeclModifierClass = TDeclModifier;

class TDeclModifier : public TObject {
protected:
    TDeclModifier *FNext;

public:
    TDeclModifier(); // test
    virtual ~TDeclModifier() override;
    virtual void __fastcall   Show(String &OutS);
    static bool __fastcall    ShowBefore();
    TDeclModifier *__fastcall GetNextOfClass(TDeclModifierClass *Cl);
    __property TDeclModifier* Next = { read = FNext, write = FNext };
};

// Representation of a string from DCU memory without copying chars
struct TMemStrRef {
protected:
    const char *FChars; // PAnsiChar
    unsigned int FLen;  // Cardinal

public:
    TMemStrRef(const char *f_chars, unsigned int f_len) : FChars(f_chars), FLen(f_len) {}
    AnsiString S() const {
        return AnsiString(FChars, FLen);
    }

    unsigned int Len() const { return FLen; } // Property
};

class TStrDeclModifier : public TDeclModifier {
public:
    // TStrDeclModifier();
    TStrDeclModifier(TMemStrRef *AMsg); // Constructor.Create
    // AnsiString __fastcall GetMsg();

    TMemStrRef *FMsg;
};

class TDeprecatedDeclModifier : public TStrDeclModifier {
public:
    TDeprecatedDeclModifier(TMemStrRef *AMsg) : TStrDeclModifier(AMsg) {}
    void __fastcall Show(String &OutS) override;
};

// XML Docs (recorded here since D 11)
class TXMLDocDeclModifier : public TStrDeclModifier {
public:
    // TXMLDocDeclModifier();
    TXMLDocDeclModifier(TMemStrRef * AMsg);
    void __fastcall Show(String &OutS) override;
    static bool __fastcall ShowBefore();
};

struct TAttributeDeclAddrArg {
    int hDT;
    int hDTAddr;
};

struct TAttributeDeclArg {
    int Kind;
    union {
        TConstValInfo C;
        TAttributeDeclAddrArg A;
    };
};

using TAttributeDeclArgs = TAttributeDeclArg[256];
using PAttributeDeclArgs = TAttributeDeclArgs *;

class TAttributeDeclModifier : public TDeclModifier {
public:
    TAttributeDeclModifier(); // Attention! In contrast to the other modifiers the constructor reads the data
    ~TAttributeDeclModifier() override;
    void __fastcall Show(String &OutS) override;
    static bool __fastcall ShowBefore();

    int hAttrDT;
    int hMember;
    int hAttrCtor;
    int ArgCnt;

    PAttributeDeclArgs Args;
};

// .Net information (was observed in DCUIL but may be used somewhere else)
// The value in the generated code
class TGeneratedNameDeclModifier : public TStrDeclModifier {
public:
    TGeneratedNameDeclModifier(TMemStrRef * AMsg);
    void __fastcall Show(String &OutS) override;
};

typedef struct {
    TMemStrRef *Name;
    int V, V1; // Unknown
    int hDT;
} TExtraProcArg, *PExtraProcArg;

using TExtraProcArgs = TExtraProcArg[256];
using PExtraProcArgs = TExtraProcArgs *;

// class PExtraProcArgs {
// public:
//     TExtraProcArg *Arg[Byte];
// };

// In DCUIL aux records are used as an owner frame for embedded subroutines.
// The records are passed as extra parameters of procedures, and the table contains info about the parameters
class TExtraArgsDeclModifier : public TDeclModifier {
public:
    TExtraArgsDeclModifier();
    ~TExtraArgsDeclModifier() override;
    // void __fastcall Read(); // Attention! In contrast to the other modifiers, the constructor reads the data
    void __fastcall Show(String &OutS) override;

    int ArgCnt;
    PExtraProcArgs Args;
};

class TTemplateParmsDeclModifier : public TDeclModifier {
public:
    TTemplateParmsDeclModifier();
    ~TTemplateParmsDeclModifier() override;
    static void __fastcall Read(TDCURec *Owner);
    void __fastcall Show(String &OutS) override;
    static bool __fastcall ShowBefore();

    TNDX    hFn; // 0 => formal parameters, else - actual parameters
    int     Cnt;
    PNDXTbl Tbl;
};

class TNameDecl : public TDCURec {
protected:
    PName __fastcall GetName() override;

public:
    TNameDecl(); // Create
    TNameDecl(bool All); // Create0 / Create00
    ~TNameDecl();
    void __fastcall  Visit(TDCURecVisitor *Visitor) override;
    void __fastcall  ShowName(String &OutS) override;
    void __fastcall  Show(String &OutS) override;
    void __fastcall  ShowConstAddInfo(String &OutS);
    void __fastcall  ShowDef(bool All, String &OutS) override;
    PName __fastcall GetExpName();
    DWord __fastcall SetMem(DWord MOfs, DWord MSz) override;
    Byte __fastcall  GetSecKind() override;
    bool __fastcall  IsVisible(Byte LK) override;
    TDCURecTag __fastcall  GetTag() override; // TDCURecTag
    void __fastcall  AddModifier(TDeclModifier *M);
    void __fastcall  ShowModifiers(bool Before);
    TDeclModifier *__fastcall GetModifierOfClass(TDeclModifierClass *Cl);
    // Create00

    PNameDef Def;
    int      hDecl;
    int      ConstAddInfoFlags; // From the corresponding ConstAddInfo

    TDeclModifier *FModifiers;
};

using PNameDecl = TNameDecl *;

class TNameFDecl : public TNameDecl {
public:
    TNameFDecl(bool NoInf);
    void __fastcall Visit(TDCURecVisitor *Visitor) override;
    void __fastcall Show(String &OutS) override;
    bool __fastcall IsVisible(Byte LK) override;
    virtual void __fastcall ShowStamps();

    TNDX F, F1;
    int Inf;
    TNDX B2; // D8+
    TNDX PkgNdx;

protected:
    void __fastcall ReadPkgNdx();
};

class TTypeDecl : public TNameFDecl {
protected:
    PName __fastcall GetName() override;
public:
    TTypeDecl();
    bool __fastcall  IsVisible(Byte LK) override;
    void __fastcall  Visit(TDCURecVisitor *Visitor) override;
    void __fastcall  Show(String &OutS) override;
    void __fastcall  EnumUsedTypes(TTypeUseAction Action, DWord *IP) override;
    DWord __fastcall SetMem(DWord MOfs, DWord MSz) override;
    Byte __fastcall  GetSecKind() override;
    void __fastcall  ShowStamps() override;

    TDefNDX hDef;
};

class TVarDecl : public TNameFDecl {
public:
    TVarDecl();
    void __fastcall Visit(TDCURecVisitor *Visitor) override;
    void __fastcall Show(String &OutS) override;
    void __fastcall EnumUsedTypes(TTypeUseAction Action, DWord *IP) override;
    Byte __fastcall GetSecKind() override;

    TDefNDX hDT;
    DWord   Ofs;
};

class TVarVDecl : public TVarDecl {
public:
    // In DXE2 win64 an auxiliary variable __puiHead has memory image
    TVarVDecl();
    void __fastcall  Visit(TDCURecVisitor *Visitor) override;
    void __fastcall  Show(String &OutS) override;
    DWord __fastcall SetMem(DWord MOfs, DWord MSz) override;

    DWord Sz;
};

class TVarCDecl : public TVarDecl {
public:
    TVarCDecl(bool OfsValid);
    void __fastcall  Visit(TDCURecVisitor *Visitor) override;
    void __fastcall  Show(String &OutS) override;
    DWord __fastcall SetMem(DWord MOfs, DWord MSz) override;
    Byte __fastcall  GetSecKind() override; // TSegKind
    void __fastcall  SetSegKind(TSegKind seg_kind) override;
    void __fastcall  SetPDataLinks(); // //Called from TVarCDecl.SetSegKind
    bool __fastcall  IsSpecialConst();

    DWord Sz; // Cardinal
    DWord OfsR; // Cardinal
    TSegKind FSeg; // For PData and XData in 64-bit mode
};

class TAbsVarDecl : public TVarDecl {
public:
    TAbsVarDecl();
    void __fastcall Visit(TDCURecVisitor *Visitor) override;
    void __fastcall Show(String &OutS) override;
};

class TTypePDecl : public TVarCDecl {
public:
    TTypePDecl();
    void __fastcall Visit(TDCURecVisitor *Visitor) override;
    void __fastcall Show(String &OutS) override;
    bool __fastcall IsVisible(Byte LK) override;
    void __fastcall ShowStamps() override;
};

class TThreadVarDecl : public TVarDecl {
public:
    TThreadVarDecl();
    Byte __fastcall GetSecKind() override;
    void __fastcall Visit(TDCURecVisitor *Visitor) override;
};

// Abstract base class - ancestor of TStrConstDecl and TProcDecl
class TMemBlockRef : public TNameFDecl {
public:
    TMemBlockRef(bool NoInf) : TNameFDecl(NoInf), Ofs(0), Sz(0) {}

    virtual void __fastcall MemRefFound(); // abstract
    void __fastcall Visit(TDCURecVisitor *Visitor) override;

    DWord Ofs;
    DWord Sz;
};

// In Delphi>=8 they started to create this kind of records for string constants
// and other data blocks (instead of TProcDecl, which was used earlier)
class TStrConstDecl : public TMemBlockRef {
public:
    TStrConstDecl();
    DWord __fastcall SetMem(DWord MOfs, DWord MSz) override;
    Byte __fastcall  GetSecKind() override;
    void __fastcall  MemRefFound() override;
    // bool __fastcall IsVisible(Byte LK);
    void __fastcall Visit(TDCURecVisitor *Visitor) override;
    void __fastcall Show(String &OutS) override;
    void __fastcall EnumUsedTypes(TTypeUseAction Action, DWord *IP) override;

    TDefNDX hDT;
    DWord   FX;
    DWord   FX1;
    bool    FMemUsed;
};

class TLabelDecl : public TNameDecl {
public:
    TLabelDecl();
    void __fastcall Visit(TDCURecVisitor *Visitor) override;
    void __fastcall Show(String &OutS) override;
    Byte __fastcall GetSecKind() override;
    bool __fastcall IsVisible(Byte LK) override;

    DWord Ofs;
};

class TExportDecl : public TNameDecl {
public:
    TExportDecl();
    void __fastcall Visit(TDCURecVisitor *Visitor) override;
    void __fastcall Show(String &OutS) override;
    Byte __fastcall GetSecKind() override;
    bool __fastcall IsVisible(Byte LK) override;

    TNDX hSym;
    TNDX Index;
};

// The common parent for TLocalDecl and TPropDecl
class TLocalDeclBase : public TNameDecl {
public:
    TLocalDeclBase();
    Byte __fastcall GetLocFlagsSecKind(); // TDeclSecKind

    TNDX    LocFlags;
    TNDX    LocFlagsX; // Ver>=8 private, protected, public, published
    TDefNDX hDT;
};

class TLocalDecl : public TLocalDeclBase {
public:
    TLocalDecl(Byte LK);
    void __fastcall   Visit(TDCURecVisitor *Visitor) override;
    void __fastcall   ShowName(String &OutS) override;
    String __fastcall GetPrefix(bool &IsConst);
    void __fastcall   Show(String &OutS) override;
    void __fastcall   EnumUsedTypes(TTypeUseAction Action, DWord *IP) override;
    // Byte __fastcall   GetLocFlagsSecKind();
    Byte __fastcall   GetSecKind() override;
    // bool __fastcall IsVisible(Byte LK);

    TNDX NdxB; // B: Byte; //Interface only
               // when LocFlagsX and lfauxPropField<>0 it is used to hold the actual field (TLocalDecl) of the reference
    TNDX Ndx;
};

class TLocalValDecl : public TLocalDecl {
public:
    TLocalValDecl(Byte lk);
    void __fastcall Show(String &OutS) override;

    // We don't set it to -1, because it can't be zero: the const always goes after argument
    TDefNDX hDeftVal; // Default value, which may be set by TSetDeftInfo
};

// TByteSet = set of Byte;
using TByteSet = System::Set<System::Byte, 0, 255>;

class TMethodDecl : public TLocalDecl {
public:
    TMethodDecl(Byte LK);
    void __fastcall Visit(TDCURecVisitor *Visitor) override;
    void __fastcall Show(String &OutS) override;

    bool InIntrf;
    TNDX  hImport;  // for property P:X read Proc{virtual,Implemented in parent class}
                    // or VProc copy of the corresponding procedure
};

class TClassVarDecl : public TLocalDecl {
public:
    TClassVarDecl(Byte LK);
    void __fastcall Visit(TDCURecVisitor *Visitor) override;
    void __fastcall Show(String &OutS) override;
    Byte __fastcall GetSecKind() override;
};

class TPropDecl : public TLocalDeclBase {
public:
    TPropDecl();
    String __fastcall PutOp(String Name, int hOp);
    void __fastcall   Visit(TDCURecVisitor *Visitor) override;
    void __fastcall   Show(String &OutS) override;
    void __fastcall   EnumUsedTypes(TTypeUseAction Action, DWord *IP) override;
    Byte __fastcall   GetSecKind() override; // TDeclSecKind

    // int LocFlags;
    // int LocFlagsX; // Ver>=8 private, protected, public, published
    // int hDT;
    TNDX Ndx;
    TNDX hIndex;
    TNDX hRead;
    TNDX hWrite;
    TNDX hStored;
    TNDX hDeft;
};

class TDispPropDecl : public TLocalDecl {
public:
    TDispPropDecl(Byte LK);
    void __fastcall Visit(TDCURecVisitor *Visitor) override;
    void __fastcall Show(String &OutS) override;
};

class TConstDeclBase : public TNameFDecl {
public:
    TConstDeclBase();
    void __fastcall ReadConstVal();
    void __fastcall ShowValue(String &OutS);
    void __fastcall Visit(TDCURecVisitor *Visitor) override;
    void __fastcall Show(String &OutS) override;
    void __fastcall EnumUsedTypes(TTypeUseAction Action, DWord *IP) override;
    Byte __fastcall GetSecKind() override;

    // Moved all this to TConstValInfo:
    // int     hDT;
    // DWord   Kind; // hX Ver>4 // 0 - scalar, 1 - string (offset=8), 2 - resourcestring, 3-float, 4 - set, [ver>=verD12] 5 - Unicode string (offset=12)
    // Byte    *ValPtr;
    // DWord   ValSz;
    // int     Val;

    TConstValInfo Value;
};

class TConstDecl : public TConstDeclBase {
public:
    TConstDecl();
    bool __fastcall IsVisible(Byte LK) override;
    void __fastcall Visit(TDCURecVisitor *Visitor) override;

    bool Adopted; // true -> it is a default value of argument
};

class TResStrDef : public TVarCDecl {
public:
    TResStrDef();
    void __fastcall Show(String &OutS) override;
    Byte __fastcall GetSecKind() override; // TDeclSecKind

    DWord OfsR;
};

class TSetDeftInfo : public TDCURec {
protected:
    bool Adopted;
public:
    TSetDeftInfo();
    void __fastcall Visit(TDCURecVisitor *Visitor) override;
    void __fastcall Show(String &OutS) override;
    bool __fastcall IsVisible(Byte LK) override;

    TDefNDX hConst, hArg;
};

class TCopyDecl : public TNameDecl {
public:
    TCopyDecl(); // Create
    void __fastcall Visit(TDCURecVisitor *Visitor) override;
    void __fastcall Show(String &OutS) override;
    Byte __fastcall GetSecKind() override; // TDeclSecKind

    TDefNDX hBase;
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

class TProcDecl : public TMemBlockRef {
public:
    TProcDecl(TDCURec *AnEmbedded, bool NoInf); // Create
    ~TProcDecl() override;
    bool __fastcall  IsUnnamed();
    DWord __fastcall SetMem(DWord MOfs, DWord MSz) override;
    Byte __fastcall  GetSecKind() override; // TDeclSecKind
    void __fastcall  ShowArgs(String &OutS, PPROCDECLINFO pInfo);
    void __fastcall  AddLocal(TDCURec *Loc);
    // IsStaticMethod
    bool __fastcall  IsProc();
    // bool __fastcall  IsProcEx(Pointer ProcUnit);
    void __fastcall  Visit(TDCURecVisitor *Visitor) override;
    // String __fastcall GetProcKindStr();
    void __fastcall  ShowDef(bool All, String &OutS) override;
    void __fastcall  Show(String &OutS) override;
    void __fastcall  EnumUsedTypes(TTypeUseAction Action, DWord *IP) override;
    bool __fastcall  IsVisible(Byte LK) override;
    // bool __fastcall GetRegDebugInfo(int ProcOfs, int hReg, int Ofs, TRegDebugInfo* Info);
    // String __fastcall GetRegDebugInfoStr(int ProcOfs, int hReg, int Ofs, int* hDecl);
    // TLocalDecl * __fastcall GetResultVar();
    void __fastcall MemRefFound() override;
    bool __fastcall IsStaticMethod();

    // DWord    CodeOfs;
    DWord    AddrBase; // May be>0 if the procedure is from a block of a *.obj file, usually AddrBase=0
    // DWord    Sz;
    TNDX     B0;
    TNDX     VProc;
    TNDX     hDTRes;
    TNDX     hClass;
    TDCURec *Locals; // FLocals
    TDCURec *Args;
    TDCURec *Embedded;
    Byte     CallKind; // TProcCallKind
    // TMethodKind
    Byte     MethodKind; // maybe this information is encoded by some flag, but
                         // I can't detect it. Maybe it would be enough to analyse the structure of
                         // the procedure name, but this way it will be safer.
    bool       OfClass;  // may be this information is encoded by some flag too, but by now it is set by the corresponding method too
    bool       JustData; // This flag is turned on by Fixups from String-typed consts
    PLocVarRec FProcLocVarTbl;
    int        FProcLocVarCnt;
    TDCURec   *FTemplateArgs;

    //   __property  TDCURec *Locals { read FLocals };

};
using PProcDecl = TProcDecl *;

class TSysProcDecl : public TNameDecl {
public:
    TSysProcDecl();
    void __fastcall Visit(TDCURecVisitor *Visitor) override;
    void __fastcall Show(String &OutS) override;
    Byte __fastcall GetSecKind() override; // TNDX

    TNDX F;
    TNDX Ndx;
};

// Starting from Delphi 8, Borland begins to give complete proc. defs to system procedures
class TSysProc8Decl : public TProcDecl {
public:
    TSysProc8Decl();
    void __fastcall Visit(TDCURecVisitor *Visitor) override;

    TNDX F;
    TNDX Ndx;
};

// Ver 7.0 and higher, MSIL
class TUnitAddInfo : public TNameFDecl {
public:
    TUnitAddInfo();
    ~TUnitAddInfo() override;
    bool __fastcall IsVisible(Byte LK) override;
    void __fastcall Visit(TDCURecVisitor *Visitor) override;

    // FVer 8.0 and higher, MSIL
    TNDX B;
    TDCURec *Sub;
};

struct TQualInfo;
using PQualInfo = TQualInfo *;

struct TQualInfo {
    void*   U;         // TUnit pointer (cast to TUnit* or keep void* if TUnit isn't declared yet)
    TNDX    hDT;       // The index of the member data type
    TNDX    hDTAddr;   // The index of the Address of the data type
    int     OfsRest;   // The remaining offset
    bool    IsVMT;     // It is the VMT offset of the data type
};

class TSpecVar : public TVarDecl {
public:
    TSpecVar();
    void __fastcall Visit(TDCURecVisitor *Visitor) override;
    void __fastcall Show(String &OutS) override;
};

enum class TTypeValKind { vkNone, vkOrdinal, vkFloat, vkStr, vkPointer, vkClass, vkInterface, vkDynArray, vkMethod, vkComplex };

class TTypeDef : public TBaseDef {
protected:
    TNDX FhDT;
public:
    TTypeDef();
    ~TTypeDef() override;
    void __fastcall           Visit(TDCURecVisitor *Visitor) override;
    void __fastcall           ShowBase();
    virtual int __fastcall    ShowValue(Byte *DP, DWord DS, String &OutS);
    void __fastcall           Show(String &OutS) override;
    DWord __fastcall          SetMem(DWord MOfs, DWord MSz) override;
    virtual String __fastcall GetOfsQualifier(int Ofs);
    virtual String __fastcall GetRefOfsQualifier(int Ofs);
    void __fastcall           AddModifier(TDeclModifier *M);
    TDeclModifier *__fastcall GetModifierOfClass(TDeclModifierClass *Cl);
    virtual TTypeValKind __fastcall ValKind() { return TTypeValKind::vkNone; }

    TNDX  RTTISz; // Size of RTTI for type, if available
    TNDX  Sz;     // Size of corresponding variable
    TNDX  hAddrDef;
    TNDX  X;
    DWord RTTIOfs;

    // Duplicates the Modifier infrastructure from TNameDecl.
    // It is required just for TTemplateParmsDeclModifier,
    // but the general implementation may become useful later
    TDeclModifier *FModifiers;

    // Aux field, to be able to get quickly the type index of the data type
    __property TNDX hDT = { read = FhDT, write = FhDT };
};

class TRangeBaseDef : public TTypeDef {
public:
    TRangeBaseDef();
    void __fastcall GetRange(PInt64Rec Lo, PInt64Rec Hi);
    bool __fastcall IsChar();
    void __fastcall Visit(TDCURecVisitor *Visitor) override;
    int __fastcall  ShowValue(Byte *DP, DWord DS, String &OutS) override;
    void __fastcall Show(String &OutS) override;
    void __fastcall EnumUsedTypes(TTypeUseAction Action, DWord *IP) override;
    TTypeValKind __fastcall ValKind() override { return TTypeValKind::vkOrdinal; }

    // TInt64Rec *__fastcall GetValCount() override;

    TNDX  hDTBase;
    Byte *LH;
    Byte  B;
};

class TRangeDef : public TRangeBaseDef {
public:
    TRangeDef();
    void __fastcall Visit(TDCURecVisitor *Visitor) override;
};

class TEnumDef : public TRangeBaseDef {
public:
    TEnumDef();
    ~TEnumDef() override;
    int __fastcall  ShowValue(Byte *DP, DWord DS, String &OutS) override;
    void __fastcall Show(String &OutS) override;
    void __fastcall Visit(TDCURecVisitor *Visitor) override;

    TNDX    Ndx;
    TList *NameTbl;
    TConstDecl *CStart; // Filled in TUnit.SetEnumConsts
    bool HasEq; // Some const was defined by �=�prev and not included into NameTbl
};

class TFloatDef : public TTypeDef {
public:
    TFloatDef();
    AnsiString __fastcall GetKindName();
    void __fastcall   Visit(TDCURecVisitor *Visitor) override;
    int __fastcall    ShowValue(Byte *DP, DWord DS, String &OutS) override;
    void __fastcall   Show(String &OutS) override;
    TTypeValKind __fastcall ValKind() override { return TTypeValKind::vkFloat; }

    Byte Kind; // TFloatKind
};

class TPtrDef : public TTypeDef {
public:
    TPtrDef();
    bool __fastcall   ShowRefValue(TNDX Ndx, DWord Ofs, String &OutS);
    void __fastcall   Visit(TDCURecVisitor *Visitor) override;
    int __fastcall    ShowValue(Byte *DP, DWord DS, String &OutS) override;
    void __fastcall   Show(String &OutS) override;
    void __fastcall   EnumUsedTypes(TTypeUseAction Action, DWord *IP) override;
    String __fastcall GetRefOfsQualifier(int Ofs) override;
    TTypeValKind __fastcall ValKind() override { return TTypeValKind::vkPointer; }

    TNDX hRefDT;
};

class TTextDef : public TTypeDef {
public:
    TTextDef();
    void __fastcall Visit(TDCURecVisitor *Visitor) override;
    void __fastcall Show(String &OutS) override;
};

class TFileDef : public TTypeDef {
public:
    TFileDef();
    void __fastcall Visit(TDCURecVisitor *Visitor) override;
    void __fastcall Show(String &OutS) override;
    void __fastcall EnumUsedTypes(TTypeUseAction Action, DWord *IP) override;

    TNDX hBaseDT;
};

class TSetDef : public TTypeDef {
public:
    TSetDef();
    void __fastcall Visit(TDCURecVisitor *Visitor) override;
    int __fastcall  ShowValue(Byte *DP, DWord DS, String &OutS) override;
    void __fastcall Show(String &OutS) override;
    void __fastcall EnumUsedTypes(TTypeUseAction Action, DWord *IP) override;
    TTypeValKind __fastcall ValKind() override { return TTypeValKind::vkComplex; }

    Byte BStart; // 0-based start byte number
    TNDX hBaseDT;
};

// This type is required to make it parent of TStringDef
class TArrayDef0 : public TTypeDef {
public:
    TArrayDef0(bool IsStr);
    void __fastcall Visit(TDCURecVisitor *Visitor) override;
    int __fastcall  ShowValue(Byte *DP, DWord DS, String &OutS) override;
    void __fastcall Show(String &OutS) override;
    void __fastcall EnumUsedTypes(TTypeUseAction Action, DWord *IP) override;

    Byte B1;
    TNDX  hDTNdx;
    TNDX  hDTEl;
};

class TArrayDef : public TArrayDef0 {
public:
    TArrayDef(bool IsStr);
    String __fastcall GetOfsQualifier(int Ofs) override;
    void __fastcall Visit(TDCURecVisitor *Visitor) override;
    TTypeValKind __fastcall ValKind() override;
};

class TShortStrDef : public TArrayDef {
public:
    TShortStrDef();
    void __fastcall Visit(TDCURecVisitor *Visitor) override;
    int __fastcall  ShowValue(Byte *DP, DWord DS, String &OutS) override;
    void __fastcall Show(String &OutS) override;
    TTypeValKind __fastcall ValKind() override { return TTypeValKind::vkStr; }

    int CP; // for Ver>=VerD12 - Code Page
};

class TStringDef : public TArrayDef {
public:
    TStringDef();
    void __fastcall   Visit(TDCURecVisitor *Visitor) override;
    bool __fastcall   ShowRefValue(TNDX Ndx, DWord Ofs, String &OutS);
    int __fastcall    ShowValue(Byte *DP, DWord DS, String &OutS) override;
    void __fastcall   Show(String &OutS) override;
    String __fastcall GetRefOfsQualifier(int Ofs) override;
    TTypeValKind __fastcall ValKind() override { return TTypeValKind::vkStr; }

    int CP; // for Ver>=VerD12 - Code Page
};

class TVariantDef : public TTypeDef {
public:
    TVariantDef();
    void __fastcall Visit(TDCURecVisitor *Visitor) override;
    void __fastcall Show(String &OutS) override;

    Byte B;
};

class TObjVMTDef : public TTypeDef {
public:
    TObjVMTDef();
    void __fastcall Visit(TDCURecVisitor *Visitor) override;
    void __fastcall Show(String &OutS) override;

    TNDX hObjDT;
    TNDX Ndx1;
};

class TRecBaseDef : public TTypeDef {
// protected:
public:
    // todo? int __fastcall GetFldOfsQualifier(int Ofs, int QSz, PQualInfo QI, int TotSize, bool Sorted, PAnsiString QS):
    String __fastcall       GetFldOfsQualifier(int Ofs, int TotSize, bool Sorted); // todo: return int?
    TLocalDecl *__fastcall  GetFldByOfs(int Ofs, int QSz, int TotSize, bool Sorted);
    TMethodDecl *__fastcall GetMethodByVMTNDX(int VMTNDX, int VMTCnt);

public:
    TRecBaseDef();
    ~TRecBaseDef() override;
    void __fastcall         ReadFields(Byte LK);
    void __fastcall         Visit(TDCURecVisitor *Visitor) override;
    int __fastcall          ShowFieldValues(Byte *DP, DWord DS, String &OutS);
    void __fastcall         EnumUsedTypes(TTypeUseAction Action, DWord *IP) override;
    virtual TNDX __fastcall GetParentType();
    TPropDecl *__fastcall   GetFldProperty(PNameDecl Fld, int hDT);
    TDCURec *__fastcall     GetMemberByNum(int Num);

    TDCURec *Fields;
};

class TRecDef : public TRecBaseDef {
public:
    TRecDef();
    void __fastcall   Visit(TDCURecVisitor *Visitor) override;
    int __fastcall    ShowValue(Byte *DP, DWord DS, String &OutS) override;
    void __fastcall   Show(String &OutS) override;
    // todo: rename to GetOfsQualifierEx():
    bool __fastcall GetOfsQualifierEx(int Ofs, int QSz, PQualInfo QI, PAnsiString QS);
    String __fastcall GetOfsQualifier(int Ofs) override;
    TTypeValKind __fastcall ValKind() override { return TTypeValKind::vkComplex; }

    Byte B2;
};

class TProcTypeDef : public TRecBaseDef {
public:
    TProcTypeDef();
    void __fastcall   Visit(TDCURecVisitor *Visitor) override;
    int __fastcall    ShowValue(Byte *DP, DWord DS, String &OutS) override;
    bool __fastcall   IsProc();
    String __fastcall ProcStr();
    void __fastcall   ShowDecl(char *Braces, String &OutS);
    void __fastcall   Show(String &OutS) override;
    void __fastcall   EnumUsedTypes(TTypeUseAction Action, DWord *IP) override;
    TTypeValKind __fastcall ValKind() override;

    TNDX    Ndx0; // B0: Byte; //Ver>2
    TNDX    hDTRes;
    Byte   *AddStart;
    DWord   AddSz; // FVer>2
    Byte    CallKind; // TProcCallKind
    PDCURec AddInfo; // for Ver>=verD2009
};

class TOOTypeDef : public TRecBaseDef {
public:
    void __fastcall Visit(TDCURecVisitor *Visitor) override;
    // TMethodDecl *__fastcall GetMethodByVMTOfs(int Ofs);
    virtual bool __fastcall hasVMT();

    TNDX hParent;
    TNDX VMCnt; // Number of virtual methods
};

class TObjDef : public TOOTypeDef {
public:
    TObjDef();
    void __fastcall   Visit(TDCURecVisitor *Visitor) override;
    int __fastcall    ShowValue(Byte *DP, DWord DS, String &OutS) override;
    void __fastcall   Show(String &OutS) override;
    void __fastcall   EnumUsedTypes(TTypeUseAction Action, DWord *IP) override;
    TNDX __fastcall   GetParentType() override;
    String __fastcall GetOfsQualifier(int Ofs) override;
    //   function GetOfsQualifierEx(Ofs,QSz: integer; QI: PQualInfo; QS: PAnsiString): Boolean; override;
    // todo? bool __fastcall GetOfsQualifierEx(int Ofs, PQualInfo QI, PAnsiString QS) override;
    bool __fastcall   hasVMT() override;

    Byte B03;
    // Byte BFE;
    // int  Ndx1;
    // Byte B00;
    TNDX VMTOfs;
    TNDX hVMT; // the TTypePDecl, which contains VMT

};

class TClassDef : public TOOTypeDef {
protected:
    void __fastcall         MarkAuxFields();
    TLocalDecl *__fastcall  GetObjFldByOfs(int Ofs, int QSz, Byte *ObjUnit);

public:
    TClassDef();
    ~TClassDef() override;
    TTypeValKind __fastcall ValKind() override { return TTypeValKind::vkClass; }
    void __fastcall         Visit(TDCURecVisitor *Visitor) override;
    int __fastcall          ShowValue(Byte *DP, DWord DS, String &OutS) override;
    void __fastcall         Show(String &OutS) override;
    TNDX __fastcall         GetParentType() override;
    String __fastcall       GetRefOfsQualifier(int Ofs) override;
    virtual void __fastcall ReadBeforeIntf();

    TNDX InstBaseRTTISz; // Size of RTTI for the type, if available
    TNDX InstBaseSz;     // Size of corresponding variable
    TNDX InstBaseV;      // hAddr of VMT
    TNDX NdxFE;          // BFE: Byte
    TNDX PropCnt;        // Ndx00a B00a: Byte
    TNDX Flags;
    TNDX ICnt;           // FVer > 2
    PNDXTbl ITbl;
};

class TMetaClassDef : public TClassDef {
public:
    TMetaClassDef();
    void __fastcall Visit(TDCURecVisitor *Visitor) override;
    void __fastcall ReadBeforeIntf() override;

    TNDX hCl;
};

class TInterfaceDef : public TOOTypeDef {
public:
    TInterfaceDef();
    TTypeValKind __fastcall ValKind() override { return TTypeValKind::vkInterface; }
    void __fastcall Visit(TDCURecVisitor *Visitor) override;
    void __fastcall Show(String &OutS) override;

    PGUID GUID;
    Byte  B;
};

class TVoidDef : public TTypeDef {
public:
    TVoidDef();
    void __fastcall Visit(TDCURecVisitor *Visitor) override;
    void __fastcall Show(String &OutS) override;
};

class TA6Def : public TDCURec {
public:
    TA6Def();
    ~TA6Def() override;
    void __fastcall Visit(TDCURecVisitor *Visitor) override;
    void __fastcall Show(String &OutS) override;

    PDCURec Args;
};

/*class TA7Def : public TDCURec {
public:
    TA7Def();
    ~TA7Def();
    void __fastcall Show(String &OutS);

    int  hClass;
    int  Cnt;
    int *Tbl;
};*/

class TDelayedImpRec : public TNameDecl {
public:
    TDelayedImpRec();
    void __fastcall Visit(TDCURecVisitor *Visitor) override;
    void __fastcall Show(String &OutS) override;

    int Inf;
    TNDX F;
};

class TORecDecl : public TNameDecl {
public:
    TORecDecl();
    ~TORecDecl() override;
    void __fastcall Visit(TDCURecVisitor *Visitor) override;
    void __fastcall Show(String &OutS) override;

    int     DW;
    Byte    B0;
    Byte    B1;
    PDCURec Args;
};

// for Ver>=VerD12
class TDynArrayDef : public TPtrDef {
public:
    TDynArrayDef();
    TTypeValKind __fastcall ValKind() override { return TTypeValKind::vkDynArray; }
    void __fastcall   Visit(TDCURecVisitor *Visitor) override;
    void __fastcall   Show(String &OutS) override;
    String __fastcall GetRefOfsQualifier(int Ofs) override;

};

// for Ver>=VerD12 - template support
class TTemplateArgDef : public TTypeDef {
public:
    TTemplateArgDef();
    ~TTemplateArgDef() override;
    void __fastcall Visit(TDCURecVisitor *Visitor) override;
    void __fastcall Show(String &OutS) override;

    int  Cnt;
    int  V5;
    PNDXTbl Tbl;
};

// for Ver>=VerD12 - template support
class TTemplateCall : public TTypeDef {
public:
    TTemplateCall();
    ~TTemplateCall() override;
    TTypeValKind __fastcall ValKind() override;
    void __fastcall         Visit(TDCURecVisitor *Visitor) override;
    void __fastcall         Show(String &OutS) override;
    int __fastcall          ShowValue(Byte *DP, DWord DS, String &OutS) override;
    void __fastcall         EnumUsedTypes(TTypeUseAction Action, DWord *IP) override;
    void __fastcall FixDTName(); // unused

    TNDX    hDT;
    int     Cnt;
    PNDXTbl Args;
    TNDX    hDTFull;

    PName OldName;   // The Name of hDT as it was shown in DCU
    PName FixedName; // The fixed name of hDT - should be freed by this object
};

using TulongTbl = DWord[65536];// array[Word] of ulong;
using PulongTbl = TulongTbl*;

class TAssemblyData : public TDCURec {
public:
    TAssemblyData();
    ~TAssemblyData() override;
    void __fastcall Visit(TDCURecVisitor *Visitor) override;
    void __fastcall Show(String &OutS) override;
    bool __fastcall IsVisible(Byte LK) override;

    TNDX       HdrSz;
    DWord      F;                // ulong
    DWord      SzPublicKey;      // ulong
    DWord      SzPublicKeyToken; // ulong
    DWord      Y;                // ulong
    TNDX       Cnt1, Cnt2, Cnt3;
    PulongTbl Tbl1, Tbl2, Tbl3, Tbl4, Tbl5, Tbl6;
    // PulongTbl *Tbl1, *Tbl2, *Tbl3, *Tbl4, *Tbl5, *Tbl6;
    Byte      *PublicKey;      // Pointer
    Byte      *PublicKeyToken; // Pointer
    Byte      *SomeData;       // Pointer

    char* AssemblyName;
    PShortName Descr;
};

// Pattern "Visitor" for TDCURec class hierarchy
// it allows us to extend the functionality of the classes somehow, for example,
// implement XML or DBMS export
class TDCURecVisitor {
protected:
    bool FVisited;

public:
    virtual void __fastcall doVisit(TDCURec *DCURec);

// protected:
    virtual void __fastcall afterVisit(TDCURec *DCURec);
    virtual void __fastcall visitDCURec(TDCURec *DCURec);
    virtual void __fastcall visitBaseDef(TBaseDef *BaseDef);
    virtual void __fastcall visitImpDef(TImpDef *ImpDef);
    virtual void __fastcall visitUnitImpDef(TUnitImpDef *UnitImpDef);
    virtual void __fastcall visitDLLImpRec(TDLLImpRec *DLLImpRec);
    virtual void __fastcall visitImpTypeDefRec(TImpTypeDefRec *ImpTypeDefRec);
    virtual void __fastcall visitNameDecl(TNameDecl *NameDecl);
    virtual void __fastcall visitNameFDecl(TNameFDecl *NameFDecl);
    virtual void __fastcall visitTypeDecl(TTypeDecl *TypeDecl);
    virtual void __fastcall visitVarDecl(TVarDecl *VarDecl);
    virtual void __fastcall visitVarVDecl(TVarVDecl *VarVDecl);
    virtual void __fastcall visitVarCDecl(TVarCDecl *VarCDecl);
    virtual void __fastcall visitAbsVarDecl(TAbsVarDecl *AbsVarDecl);
    virtual void __fastcall visitTypePDecl(TTypePDecl *TypePDecl);
    virtual void __fastcall visitThreadVarDecl(TThreadVarDecl *ThreadVarDecl);
    virtual void __fastcall visitMemBlockRef(TMemBlockRef *MemBlockRef);
    virtual void __fastcall visitStrConstDecl(TStrConstDecl *StrConstDecl);
    virtual void __fastcall visitLabelDecl(TLabelDecl *LabelDecl);
    virtual void __fastcall visitExportDecl(TExportDecl *ExportDecl);
    virtual void __fastcall visitLocalDecl(TLocalDecl *LocalDecl);
    virtual void __fastcall visitMethodDecl(TMethodDecl *MethodDecl);
    virtual void __fastcall visitClassVarDecl(TClassVarDecl *ClassVarDecl);
    virtual void __fastcall visitPropDecl(TPropDecl *PropDecl);
    virtual void __fastcall visitDispPropDecl(TDispPropDecl *DispPropDecl);
    virtual void __fastcall visitConstDeclBase(TConstDeclBase *ConstDeclBase);
    virtual void __fastcall visitConstDecl(TConstDecl *ConstDecl);
    virtual void __fastcall visitResStrDef(TResStrDef *ResStrDef);
    virtual void __fastcall visitSetDeftInfo(TSetDeftInfo *SetDeftInfo);
    virtual void __fastcall visitCopyDecl(TCopyDecl *CopyDecl);
    virtual void __fastcall visitProcDecl(TProcDecl *ProcDecl);
    virtual void __fastcall visitSysProcDecl(TSysProcDecl *SysProcDecl);
    virtual void __fastcall visitSysProc8Decl(TSysProc8Decl *SysProc8Decl);
    virtual void __fastcall visitUnitAddInfo(TUnitAddInfo *UnitAddInfo);
    virtual void __fastcall visitSpecVar(TSpecVar *SpecVar);
    // types
    virtual void __fastcall visitTypeDef(TTypeDef *TypeDef);
    virtual void __fastcall visitRangeBaseDef(TRangeBaseDef *RangeBaseDef);
    virtual void __fastcall visitRangeDef(TRangeDef *RangeDef);
    virtual void __fastcall visitEnumDef(TEnumDef *EnumDef);
    virtual void __fastcall visitFloatDef(TFloatDef *FloatDef);
    virtual void __fastcall visitPtrDef(TPtrDef *PtrDef);
    virtual void __fastcall visitTextDef(TTextDef *TextDef);
    virtual void __fastcall visitFileDef(TFileDef *FileDef);
    virtual void __fastcall visitSetDef(TSetDef *SetDef);
    virtual void __fastcall visitArrayDef0(TArrayDef0 *ArrayDef0);
    virtual void __fastcall visitArrayDef(TArrayDef *ArrayDef);
    virtual void __fastcall visitShortStrDef(TShortStrDef *ShortStrDef);
    virtual void __fastcall visitStringDef(TStringDef *StringDef);
    virtual void __fastcall visitVariantDef(TVariantDef *VariantDef);
    virtual void __fastcall visitObjVMTDef(TObjVMTDef *ObjVMTDef);
    virtual void __fastcall visitRecBaseDef(TRecBaseDef *RecBaseDef);
    virtual void __fastcall visitRecDef(TRecDef *RecDef);
    virtual void __fastcall visitProcTypeDef(TProcTypeDef *ProcTypeDef);
    virtual void __fastcall visitOOTypeDef(TOOTypeDef *OOTypeDef);
    virtual void __fastcall visitObjDef(TObjDef *ObjDef);
    virtual void __fastcall visitClassDef(TClassDef *ClassDef);
    virtual void __fastcall visitMetaClassDef(TMetaClassDef *MetaClassDef);
    virtual void __fastcall visitInterfaceDef(TInterfaceDef *InterfaceDef);
    virtual void __fastcall visitVoidDef(TVoidDef *VoidDef);
    virtual void __fastcall visitA6Def(TA6Def *A6Def);
    virtual void __fastcall visitDelayedImpRec(TDelayedImpRec *DelayedImpRec);
    virtual void __fastcall visitORecDecl(TORecDecl *ORecDecl);
    virtual void __fastcall visitDynArrayDef(TDynArrayDef *DynArrayDef);
    virtual void __fastcall visitTemplateArgDef(TTemplateArgDef *TemplateArgDef);
    virtual void __fastcall visitTemplateCall(TTemplateCall *TemplateCall);
    virtual void __fastcall visitAssemblyData(TAssemblyData *AssemblyData);
};

//------------------------------------------------------------------------------
#endif
