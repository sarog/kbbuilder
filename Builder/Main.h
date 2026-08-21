#ifndef MAIN_H
#define MAIN_H
//------------------------------------------------------------------------------
#define LINUX   0
#define SHOW    0
#include <stdio.h>
#include "DCUClasses.h"
#include "StackTrace.h"
//------------------------------------------------------------------------------
#define     OutLog1(a)          if (fLog) fprintf(fLog, (a))
#define     OutLog2(a, b)       if (fLog) fprintf(fLog, (a), (b))
#define     OutLog3(a, b, c)    if (fLog) fprintf(fLog, (a), (b), (c))
#define     lfauxPropField      0x80000000
// #define DUMP_BEG    0x80
// #define FIXUP_BEG   0x81
//------------------------------------------------------------------------------
int __fastcall          AddAddrDef(TDCURec* ND);
TNDX __fastcall         AddTypeDef(TTypeDef* TD);
void __fastcall         AddTypeName(int hDef, int hDecl, PName Name);
PName __fastcall        AllocName(const AnsiString S); // unused
void __fastcall         FreeName(PName name);
AnsiString __fastcall   CharStr(char Ch);
void __fastcall         ClearAddrDef(TNameDecl *ND);
void __fastcall         ClearLastTypeDef(TTypeDef *TD);
TDCURecTag __fastcall   FixTag(TDCURecTag recTag);
void __fastcall         RegisterEmbeddedTypes(TDCURec* Embedded, int Depth);
void __fastcall         BindEmbeddedType(PDCURec UseRec, int hDT, DWord* IP);
void __fastcall         BindEmbeddedTypes();
void __fastcall         ChkSize(Cardinal Sz);
TDCURec* __fastcall     ConsumeEmbedded();
void __fastcall         EnumUsedTypeList(PDCURec L, TTypeUseAction Action, DWord* IP);
void __fastcall         FreeDCURecList(TDCURec* L);
TDCURec* __fastcall     GetAddrDef(int hDef);
PName __fastcall        GetAddrName(int hDef);
AnsiString __fastcall   GetAddrStr(int hDef, bool ShowNDX); // was String
Byte* __fastcall        GetBlockMem(DWord BlOfs, DWord BlSz, DWord* ResSz);
AnsiString __fastcall   GetDCURecStr(TDCURec* D, int hDef); // was String
TDCURec* __fastcall     GetGlobalAddrDef(int hDef);
TTypeDef * __fastcall   GetLastAddedTypeDef();
TTypeDef* __fastcall    GetLocalTypeDef(int hDef);
TTypeDef* __fastcall    GetGlobalTypeDef(int hDef);
TTypeValKind __fastcall GetGlobalTypeValKind(int hDT);
TSegKind __fastcall     GetSegKindByName(PName Name);
int __fastcall          GetStartFixup(DWord Ofs);
PName __fastcall        GetTypeName(int hDef);
TTypeDef* __fastcall    GetTypeDef(int hDef);
int __fastcall          GetTypeSize(int hDef);
PUnitImpRec __fastcall  GetUnitImpRec(int hUnit);
PDCURec __fastcall      IncEmbedDepth(TDCURec *&HeadBuf);
void __fastcall         LoadAddrToSegInfo();
bool __fastcall         MemToUInt(Byte *DP, DWord Sz, DWord* Res);
AnsiString __fastcall   NDXToStr(int NDXLo); // was String
AnsiString __fastcall   PName2String(PName Name); // was String
Byte __fastcall         ReadByte();
void __fastcall         ReadByteIfEQ(Byte V);
int __fastcall          ReadByteFrom(bool b);
int __fastcall          ReadByteFrom(TByteSet S);
Byte __fastcall         ReadCallKind();
int __fastcall          ReadClassInterfaces(PPNDXTbl PITbl);
int __fastcall          ReadConstAddInfo(TNameDecl* LastProcDecl);
void __fastcall         ReadDeclList(Byte LK, TDCURec *Owner, TDCURec **Result);
void __fastcall         ReadDependencyInfo();
int __fastcall          ReadIndex();
void __fastcall         ReadIndex64(PInt64Rec Res);
Byte* __fastcall        ReadMem(DWord Sz);
PShortName __fastcall   ReadShortName();
PName __fastcall        ReadName();
TMemStrRef *__fastcall  ReadNDXStrRef();
void __fastcall         ReadInDcpWin64Info();
void __fastcall         ReadSomeNameInfo28();
TDCURecTag __fastcall   ReadTag();
int __fastcall          ReadUIndex();
void __fastcall         ReadUIndex64(PInt64Rec Res);
DWord __fastcall        ReadULong();
int __fastcall          AppendAddrDef(TDCURec *ND);
void __fastcall         RefAddrDef(int V);
bool __fastcall         RegTypeShow(TBaseDef* T);
void __fastcall         RestoreFixupMemState(TFixupMemState* S);
void __fastcall         SaveFixupMemState(TFixupMemState* S);
void __fastcall         SetUnitPackageInfo(int hDecl, String sInfo);
void __fastcall         SetStartFixupInfo(int Fix0);
void __fastcall         SetCodeRange(Byte* ACodeStart, Byte* ACodeBase, DWord ABlSz);
void __fastcall         SetProcAddInfo(int V);
void __fastcall         ShowDataBl(DWord Ofs0, DWord BlOfs, DWord BlSz);
void __fastcall         ShowDataBlP(Byte *DP, DWord DS, DWord Ofs0);
void __fastcall         ShowCodeBl(DWord Ofs0, DWord BlOfs, DWord BlSz);
void __fastcall         ShowDeclList(Byte LK, TDCURec* Decl, String& OutS);
void __fastcall         ShowDump(Byte* DP, Byte* DPFile0, DWord FileSize, DWord SizeDispl, DWord Size, DWord Ofs0Displ, DWord Ofs0, DWord WMin, int FixCnt, TFixupRec* FixTbl);
bool __fastcall         ShowGlobalConstValue(int hDef, String& OutS);
String __fastcall       ShowOfsQualifier(int hDef, int Ofs);
int __fastcall          ShowGlobalTypeValue(int hDef, Byte *DP, DWord DS, bool AndRest, int ConstKind, bool IsNamed, String &OutS);
int __fastcall          ShowStrConst(Byte* DP, DWord DS, String& OutS);
int __fastcall          ShowUnicodeStrConst(Byte* DP, DWord DS, String& OutS); // Ver >=verD12
int __fastcall          ShowUnicodeResStrConst(Byte* DP, DWord DS, String& OutS); // Ver >=verD12
String __fastcall       ShowRefOfsQualifier(int hDef, int Ofs);
String __fastcall       ShowTypeDef(int hDef, PName N);
String __fastcall       ShowTypeName(int hDef); // bool
int __fastcall          ShowTypeValue(TTypeDef *T, Byte *DP, DWord DS, int ConstKind, bool IsNamed, String &OutS);
void __fastcall         SkipBlock(Cardinal Sz);
AnsiString __fastcall   StrConstStr(char* CP, int L); // was String
bool __fastcall         TypeIsVoid(int hDef);
void __fastcall         UnRegTypeShow(TBaseDef* T);
void __fastcall         VisitDeclList(TDCURecVisitor *Visitor, Byte LK, TDCURec *MainRec, TDCURec* Decl);

void __fastcall VisitDecls(TDCURecVisitor *Visitor, bool InterfaceOnly);
void __fastcall VisitTypes(TDCURecVisitor *Visitor);

void __fastcall ChangeScanState(TScanState State, Byte *DP, DWord MaxSz);
void __fastcall RestoreScanState(TScanState State);

//------------------------------------------------------------------------------
// OffsetsInfo
typedef struct {
    DWord Offset;
    DWord Size;
    int   ModId; // Modules
    int   NamId; // Names
} OFFSETSINFO, *POFFSETSINFO; // IDR

// Module info
typedef struct {
    int          ID;
    Word         ModuleID;
    DWord        Offset;
    DWord        Size;
    String       Name;     // Unit Name
    String       Filename; // Unit Filename
    TStringList *UsesList; // List of Uses
} MODULEINFO, *PMODULEINFO; // IDR

// Fixup info (from KB.pas)
typedef struct {
    Byte   Type; // A-ADR;J-JMP;D-DAT
    DWord  Ofs;  // Offset from RTTI data begin
    String Name; // Name
} FIXUPINFO, *PFIXUPINFO; // IDR - FIXUP_INFO

// ConstInfo (from KB.pas)
#define CI_CONSTDECL    'C'
#define CI_PDECL        'P'
#define CI_VARCDECL     'V'
typedef struct {
    int    ID;
    DWord  Offset;
    DWord  Size;
    bool   Skip;
    Word   ModuleID;
    String Name;
    Byte   Type; // look above
    String TypeDef;
    String Value;
    DWord  RTTISz;  // Size of RTTI data
    DWord  RTTIOfs; // Offset of RTTI data
    TList *Fixups;  // If VMT
} CONSTINFO, *PCONSTINFO; // IDR

// TypeInfo
typedef struct {
    int    ID;
    DWord  Offset;
    DWord  Size;
    Word   ModuleID;
    String Name;
    Byte   Kind;
    Word   VMCnt;   // Number of class VM
    DWord  RTTISz;  // Size of RTTI data
    DWord  RTTIOfs; // Offset of RTTI data
    String Decl;
    TList *Fixups;
    TList *Fields;     // List of Fields
    TList *Properties; // List of Properties
    TList *Methods;    // List of Methods
} TYPEINFO, *PTYPEINFO; // IDR

// VarInfo
#define VI_VAR          'V'
#define VI_ABSVAR       'A'
#define VI_SPECVAR      'S'
#define VI_THREADVAR    'T'
typedef struct {
    int    ID;
    DWord  Offset;
    DWord  Size;
    Word   ModuleID;
    String Name;
    Byte   Type;    // look above
    DWord  DumpOfs; // Offset of binary data
    DWord  DumpSz;  // Size of binary data
    String AbsName;
    String TypeDef;
} VARINFO, *PVARINFO; // IDR

// ResourseStringInfo (from KB.pas)
typedef struct {
    int    ID;
    DWord  Offset;
    DWord  Size;
    Word   ModuleID;
    String Name;
    DWord  DumpOfs; // Offset of binary data
    DWord  DumpSz;  // Size of binary data
    String TypeDef;
    String Context; // Context of ResStr
} RESSTRINFO, *PRESSTRINFO; // IDR

// LocalDeclInfo
typedef struct {
    Byte   Scope;
    Byte   Tag;
    int    LocFlags;
    int    Ndx;
    int    NdxB;
    int    Case; // for case
    String Name;
    String TypeDef;
    String AbsName;
} LOCALDECLINFO, *PLOCALDECLINFO; // IDR

// PropertyInfo
typedef struct {
    Byte   Scope;
    int    Index;
    int    DispId;
    String Name;
    String TypeDef;
    String ReadName;
    String WriteName;
    String StoredName;
} PROPERTYINFO, *PPROPERTYINFO; // IDR

// MethodDeclInfo
typedef struct {
    Byte   Scope;
    Byte   MethodKind; // 'M'-method,'P'-procedure,'F'-function,'C'-constructor,'D'-destructor
    String Prototype;
} METHODDECLINFO, *PMETHODDECLINFO; // IDR

//------------------------------------------------------------------------------
// From FixUp.pas
using TFxSizeTbl = SmallInt[fxMax + 1];

//------------------------------------------------------------------------------

// todo?
class TPDataIterator;

// From Win64SEH.pas
class TWin64UnwindInfo : public TObject {
protected:
    Byte* PDataDP; // Pointer
    DWord PDataDS; // Cardinal
    Byte* DP; // Pointer
    DWord DS; // Cardinal
    TNDX UnwindNdx, ProcNdx; // TNDX
    TObject* UnwindDR, *ProcDR;
    PFixupTbl FPDataFixTbl;

public:
    void Clear();
    bool Init0(Byte* APDataDP, DWord APDataDS, Byte* ADP, DWord ADS);
    bool InitXData(unsigned long hPData, void* ADP, unsigned long ADS);
    bool InitPData(unsigned long hPData) { return true; } // todo
    bool Show();
    void SetPDataLinks(unsigned long hDecl) {} // todo
    bool FirstPDataRec(TPDataIterator& Iter);
    bool NextPDataRec(TPDataIterator& Iter);

    // TPDataIterator nested class (moved out for C++ compatibility)
    class TPDataIterator {
    protected:
        int Ofs;
        // PFixupTbl PDataFixTbl;
        // TFixUpReader FixRd;
        unsigned long CurOfs;
        // PUNWIND_INFO UI;
        unsigned long UIExcCnt;

    public:
        // PPDataRec DR;
        unsigned long hProc, hUnwind;

        // ExcScope section
        // PExcScope ExcScope;
        unsigned long hScopeProc, hScopeTable, hScopeTarget;
        bool NextExcScope();

    private:
        // ExcDesc section
        int ExcDescRest;
        // TFixUpReader ExcFixRd;

    public:
        int ExcDescCnt;
        // PExcDescEntry ExcDesc;
        unsigned long hExcVTable, hExcHandler;
        bool NextExcDesc();
    };
};

typedef TWin64UnwindInfo *PWin64UnwindInfo;
//------------------------------------------------------------------------------

#endif
/*
TDCURec [Next]
    *TNameDecl [Def, hDecl]
        *TNameFDecl [F, Inf, B2]
            *TTypeDecl [hDef]
            *TVarDecl [hDT, Ofs]
                *TVarCDecl [Sz, OfsR]
                    *TTypePDecl
                    *TResStrDef [OfsR]
                TAbsVarDecl
                TThreadVarDecl
                TSpecVar
            *TStrConstDecl [hDT, Ofs, Sz]
            *TConstDeclBase [hDT, hX, ValPtr, ValSz, Val]
                *TConstDecl
            *TProcDecl [CodeOfs, AddrBase, Sz, B0, VProc, hDTRes, Args, Locals, Embedded, CallKind, MethodKind, JustData, FProcLocVarTbl, FProcLocVarCnt]
                *TSysProc8Decl [F, Ndx]
            *TUnitAddInfo [B, Sub]
        *TLabelDecl [Ofs]
        *TExportDecl [hSym, Index]
        *TLocalDecl [LocFlags, hDT, NdxB, Ndx]
            *TMethodDecl [InIntrf, hImport]
            TClassVarDecl
            TDispPropDecl
        *TPropDecl [LocFlags, hDT, Ndx, hIndex, hRead, hWrite, hStored, hDeft]
        *TSetDeftInfo [hConst, hArg]
        *TCopyDecl [hBase, Base]
        *TSysProcDecl [F, Ndx]
    *TBaseDef [FName, Def, hUnit]
        *TImpDef [ik, FNameIsUnique, Inf]
        *TDLLImpRec [Ndx]
        *TImpTypeDefRec [RTTIOfs, RTTISz, hImpUnit, ImpName]
        *TTypeDef [RTTISz, Sz, V, RTTIOfs]
            TRangeBaseDef [hDTBase, LH, B]
                *TRangeDef
                *TEnumDef [Ndx, NameTbl]
            *TFloatDef [B]
            *TPtrDef [hRefDT]
            TTextDef
            *TFileDef [hBaseDT]
            *TSetDef [BStart, hBaseDT]
            *TArrayDef [B1, hDTNdx, hDTEl]
                TShortStrDef
                TStringDef
            *TVariantDef [B]
            *TObjVMTDef [hObjDT, Ndx1]
            TRecBaseDef [Fields]
                *TRecDef [B2]
                *TProcTypeDef [Ndx0, hDTRes, AddStart, AddSz, CallKind]
                *TObjDef [B03, hParent, BFE, Ndx1, B00]
                *TClassDef [hParent, InstBaseRTTISz, InstBaseSz, InstBaseV, VMCnt, NdxFE, Ndx00a, B04, ICnt, ITbl]
                    TMetaClassDef [hCl]
                *TInterfaceDef [hParent, VMCnt, GUID, B]
            TVoidDef
*/