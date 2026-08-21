//------------------------------------------------------------------------------
#include <vcl.h>
#include <dir.h>
#include <stdio.h>
#include <filesystem>
#pragma hdrstop
#include <map>
#include <string>
#include <algorithm>
#include <cctype>
#include "Main.h"

#include "InputParser.h"
// #include <trace/trace.hpp>
// #include <cpptrace/cpptrace.hpp>
//------------------------------------------------------------------------------
extern String RegName[7];
extern String CallKindName[5];
//------------------------------------------------------------------------------
#define     NEW_VERSION
//------------------------------------------------------------------------------
Byte        ActiveInfo, ActiveScope;
DWord      *pDumpOffset    = nullptr;
DWord      *pDumpSize      = nullptr;
TList      *FixupsList     = nullptr; // List of Fixups
TList      *FieldsList     = nullptr; // List of Fields ("field")
TList      *PropertiesList = nullptr; // List of Properties ("property")
TList      *MethodsList    = nullptr; // List of Methods
TList      *ArgsList       = nullptr; // ("var","val")
TList      *LocalsList     = nullptr; // List of local vars ("local","local absolute", "result")
FILE       *fOut;
FILE       *fLog       = 0;
bool        ThreadVar  = false;
TList      *ModuleList = nullptr; // List of Modules
PMODULEINFO ModuleInfo = nullptr;
Word        ModuleID;
TList      *ConstList  = nullptr; // List of Constants
TList      *TypeList   = nullptr; // List of Types
TList      *VarList    = nullptr; // List of Vars
TList      *ResStrList = nullptr; // List of ResourceStrings
TList      *ProcList   = nullptr; // List of Procedures
// long        CurOffset  = 0L;    // Unused
int         CaseN      = -1; // Number of current cases

TList *FSrcFiles = nullptr; // List of Source files (re: PSrcFileRec)

// was: TShortString NoName = {1, "?"};

bool         GenVarCAsVars = false;
bool         FromPackage   = false;
int          FVer          = 0;
int          FPtrSize      = 4;
int          FPlatform     = dcuplWin32;
TDCUPlatform FPlatform2    = TDCUPlatform::Win32; // todo

int    FEmbedDepth    = 0;
int    FMaxEmbedDepth = 0;
int    FEmbedLimit    = 0;
TList *FEmbeddedTypes = nullptr; // contains embedding depths // contains PEmbeddedTypeInf, it is indexed by TD.hDef, not by FEmbedDepth

bool        IsMSIL;
bool        IsDelphi = false;
bool        IsKylix  = false;
int         NDXHi;

Byte        fxStart   = fxStart30;
Byte        fxEnd     = fxEnd30;
Byte        fxJmpAddr = fxJmpAddr0;

// for the two phase loading mode
Byte        *FMemPtr  = nullptr; // DCUData
TIncPtr      CurPos   = nullptr; // Pointer (TIncPtr/PAnsiChar) to DCUData (Current Scan State Position)
Byte        *DefStart = nullptr; // Start of definition (DCU_In.pas, Pointer)
TScanState   ScSt;
Byte         Tag;       // TDCURecTag
DWord        Magic;     // LongInt
DWord        FMemSize;  // Cardinal
DWord        FileSizeH; // ulong
TDCUFileTime FT;        // FileTime (int)
DWord        Stamp;
AnsiString   UnitName;  // FUnitName // was String
DWord        Flags;     // UnitFlags (FFlags)
DWord        UnitPrior; // UnitPriority

TList       *FUnitImp = nullptr; // List of Unit imports
TList       *FTypes   = nullptr; // List of Types
TList       *FAddrs   = nullptr; // List of Addrs
int          FhNextAddr;         // Required for ProcAddInfo in FVer>verD8
TStringList *FExportNames   = nullptr;
TDCURec     *FDecls         = nullptr; // DeclList
TDCURec     *FOtherRecords  = nullptr;
int          FTypeDefCnt    = 0;    //  FTypes definition count
TList       *FTypeShowStack = nullptr;
Byte        *FDataBlPtr     = nullptr; // TIncPtr
DWord        FDataBlSize;
DWord        FDataBlOfs;

// Fixups
int        FFixupCnt;
TFixupRec *FFixupTbl = nullptr;
int        FCodeLineCnt;
typedef Byte TByte4[4];

typedef struct {
    int ofs;
    int L;
} TCodeLineRec, *PCodeLineRec;

typedef TCodeLineRec TCodeLineTbl[256];
typedef TCodeLineTbl *PCodeLineTbl;
PCodeLineTbl FCodeLineTbl;

//Using the $I directive DCU file can be composed of several source files
//The TLineRangeRec structure represents the mapping of DCU internal line numbers
//to the real line numbers of source files
// PLineRangeRec = ^TLineRangeRec;
// TLineRangeRec = record
//   Line0,LineNum,Num0: integer;
// SrcF: PSrcFileRec;
// end ;
//
// PLineRangeTbl = ^TLineRangeTbl;
// TLineRangeTbl = array[Word] of TLineRangeRec;
// PLineRangeTbl FLineRangeTbl;

int         FLineRangeCnt;

int         FLocVarSize; // The actual number of records in FLocVarTbl (required for XE2 64bit)
int         FLocVarCnt;
PLocVarRec  FLocVarTbl = nullptr;

// TDCPLoadState = (dcplNotLoaded, dcplHeaderLoaded, dcplAllLoaded);
bool        FLoaded; // TDCPLoadState
String      FFName ; // for the two phase loading mode (DCP.pas)
String      FFExt ;

typedef struct {
    TDCURec *List;
    PDCURec ListEnd;
} TEmbeddedListInf, *PEmbeddedListInf;

using TEmbeddedListInfTbl = TEmbeddedListInf[256]; // array[Byte] = 256 elements
using PEmbeddedListInfTbl = TEmbeddedListInfTbl *;
PEmbeddedListInfTbl FEmbeddedLists; // contains the lists which were not consumed

TDCURec *CurMainRec = nullptr;
TDCURec *CurDecList = nullptr;

int FSegCnt;
PSegKindTbl FSegKindTbl = nullptr;

//------------------------------------------------------------------------------

const System::SmallString<1> NoNameStr = "?";
TNameRec NoName = *reinterpret_cast<const TNameRec*>(&NoNameStr);

//------------------------------------------------------------------------------

const TFxSizeTbl fxSizeXE64 = {
    /* 0:  */  0,  0, -1, -1,  4,  4,  4, -1,
    /* 8:  */  4, -1, -1, -1, -1, -1,  8, -1,
    /* 10: */ -1,  8, -1 /*4 for 32-bit*/, 8, 0, 4, -1, 4
};

const TFxSizeTbl fxSizeXE32 = {
    /* 0:  */  0,  0, -1, -1,  4,  4,  4, -1,
    /* 8:  */  4, -1, -1, -1, -1, -1,  8, -1,
    /* 10: */ -1, -1,  4, -1, -1, -1, -1, -1
};

System::Set<std::uint8_t, 0, fxMax> fxValid = System::Set<std::uint8_t, 0, fxMax>()
    << 0 << (fxStart30 - 1); // Range operator << for Set

TFxSizeTbl fxSize;
bool fx8Byte = false;

//------------------------------------------------------------------------------
AnsiString __fastcall PName2String(PName Name) {
    return Name->GetStr();
}
//------------------------------------------------------------------------------
bool __fastcall TypeIsVoid(int hDef) {
    if (hDef <= 0 || hDef > FTypes->Count) return true;
    TBaseDef *D = (TBaseDef *) FTypes->Items[hDef - 1];
    if (!D) return true;
    return (D->ClassType() == __classid(TVoidDef));
}
//------------------------------------------------------------------------------
PUnitImpRec __fastcall GetUnitImpRec(int hUnit) {
    return (PUnitImpRec) (FUnitImp->Items[hUnit]);
}
//------------------------------------------------------------------------------
// todo:
void __fastcall GetUnitImp(int hUnit) {
    PUnitImpRec UI = GetUnitImpRec(hUnit);

    /*// auto Result;

    if (!UI) return nullptr;

    Result = UI->U;
    if (!Result) {
        if (Integer(Result) =- 1) {
            return nullptr;
        }
    }
    Result = GetDCUByName(UI->Name->GetStr(),FFExt,FVer,IsMSIL,FPlatform,UI->Ref->Inf);
    if (!Result) then
      PtrInt(UI->U) = -1;
    else
        UI->U = Result;*/
}
//------------------------------------------------------------------------------

/**
 *
 * @param D
 * @param hDef
 * @return
 */
AnsiString __fastcall GetDCURecStr(TDCURec* D, int hDef) {
    // todo: review:
    PName  N;
    char   Pfx[32];
    String Result;

    // new:
    // if (!D) N = GetNoName();
    // else N = D->Name;

    // was:
    if (!D) N = &NoName;
    else N = D->Name;

    if (N->IsEmpty()) {
    // was: if (!N->Len) {
        strcpy(Pfx, "_N%_");
        Result = Sysutils::Format("_%x", ARRAYOFCONST((hDef)));
    // was: } else if (N->Name[0] == '.') {
    // new:
    } else if (N->Get1stChar() == '.') { // } else if (N->Name[0] == '.') {
        strcpy(Pfx, "_D%_");
        Result = PName2String(N).SubString(2, 255);
    } else {
        Pfx[0] = 0;
        Result = PName2String(N);
    }

    if (!strcmp(Pfx, "")) {
        char *CP = StrScan(Pfx, '%');
        if (CP) {
            char ScopeCh;
            if (!D)
                ScopeCh = 'N';
            else {
                if (D->InheritsFrom(__classid(TTypeDecl)) || D->InheritsFrom(__classid(TTypeDef)))
                    ScopeCh = 'T';
                else if (D->InheritsFrom(__classid(TVarDecl)))
                    ScopeCh = 'V';
                else if (D->InheritsFrom(__classid(TConstDecl)))
                    ScopeCh = 'C';
                else if (D->InheritsFrom(__classid(TProcDecl)))
                    ScopeCh = 'F';
                else if (D->InheritsFrom(__classid(TLabelDecl)))
                    ScopeCh = 'L';
                else if (D->InheritsFrom(__classid(TPropDecl)) || D->InheritsFrom(__classid(TDispPropDecl)))
                    ScopeCh = 'P';
                else if (D->InheritsFrom(__classid(TLocalDecl)))
                    ScopeCh = 'v';
                else if (D->InheritsFrom(__classid(TMethodDecl)))
                    ScopeCh = 'M';
                else if (D->InheritsFrom(__classid(TExportDecl)))
                    ScopeCh = 'E';
                else
                    ScopeCh = 'n';
            }
            do {
                *CP = ScopeCh;
                CP  = StrScan(CP + 1, '%');
            } while (CP);
        }
        Result = String(Pfx) + Result;
    }
    return Result;
}
//------------------------------------------------------------------------------
void __fastcall ChkListSize(TList *L, int hDef) {
    if (hDef <= 0) return;
    if (hDef > L->Count) {
        if (hDef > L->Capacity)
            L->Capacity = (hDef * 3) / 2;
        L->Count = hDef;
    }
}
//------------------------------------------------------------------------------
// Two methods against circular references
bool __fastcall RegTypeShow(TBaseDef *T) {
    if (FTypeShowStack->IndexOf(T) >= 0) return false;
    FTypeShowStack->Add(T);
    return true;
}
//------------------------------------------------------------------------------
void __fastcall UnRegTypeShow(TBaseDef *T) {
    int C = FTypeShowStack->Count - 1;
    FTypeShowStack->Count = C;
}
//------------------------------------------------------------------------------

/**
 * TUnit.AddTypeDef
 * @param TD
 * @return TNDX
 */
TNDX __fastcall AddTypeDef(TTypeDef* TD) {
    ChkListSize(FTypes, FTypeDefCnt + 1);
    TBaseDef *BD = (TBaseDef*)FTypes->Items[FTypeDefCnt];
    if (BD) {
        if (BD->Def)
            OutLog2("[Error]: Type def #%lX override\n", FTypeDefCnt + 1); // DCUErrorFmt
        if (BD->hUnit != TD->hUnit)
            OutLog2("[Error]: Type def #%lX unit mismatch\n", FTypeDefCnt + 1); // DCUErrorFmt

        TD->FName  = BD->Name;
        TD->hDecl = BD->hDecl;
        BD->FName = nullptr;
        delete BD;
    }
    FTypes->Items[FTypeDefCnt] = TD;
    auto Result = FTypeDefCnt; // for TUnit.*
    FTypeDefCnt++;
    return Result;
}
//------------------------------------------------------------------------------

/**
 * TUnit.SetListDefName
 * @param L
 * @param hDef
 * @param hDecl
 * @param Name
 */
void __fastcall SetListDefName(TList* L, int hDef, int hDecl, PName Name) {
    if (!L) return;
    if (hDef <= 0) return;
    ChkListSize(L, hDef);
    hDef--;
    TBaseDef *Def = (TBaseDef *) (L->Items[hDef]);
    if (!Def) {
        Def = new TBaseDef(Name, nullptr, -1);
        L->Items[hDef] = static_cast<void *>(Def);
        Def->hDecl = hDecl;
        return;
    }
    if (!Def->FName) {
        Def->FName = Name;
    } else if (!Name->IsAuxName()) {
        if (Def->FName->IsAuxName()) {
            Def->FName = Name;
        }
    }

    if (!Def->hDecl)
        Def->hDecl = hDecl;
}
//------------------------------------------------------------------------------
void __fastcall AddTypeName(int hDef, int hDecl, PName Name) {
    SetListDefName(FTypes, hDef, hDecl, Name);
}
//------------------------------------------------------------------------------

/**
 * From DCU_IN.pas. Currently unused; only referenced from FixDTName()
 *
 * @param S
 * @return
 */
PName __fastcall AllocName(const AnsiString S) {
    PName Result = nullptr;
    int L = S.Length();

    if (L >= 255) {
        if (FVer >= verDXE2 && FVer < verK1) {
            // GetMem(Result,L*SizeOf(AnsiChar)+SizeOf(Byte)+SizeOf(LongInt));
            Result = static_cast<PName>(GetMemory(L * sizeof(AnsiChar) + sizeof(Byte) + sizeof(DWord)));
            Result->D.bLen = 0xFF;
            Result->D.dwLen = L;
            System::Move(S.c_str(), Result->D.lS, L * sizeof(AnsiChar));

            return Result;
        }
        L = 255;
    }

    Result = static_cast<PName>(GetMemory((L + 1) * sizeof(AnsiChar)));
    Result->D.S = System::ShortString(S);

    return Result;
}
//------------------------------------------------------------------------------
void __fastcall FreeName(PName NP) {
    if (!NP)
        return;
    FreeMemory(NP);
}
//------------------------------------------------------------------------------
// Cardinal
void __fastcall SkipBlock(Cardinal Sz) {
    CurPos += Sz;
}
//------------------------------------------------------------------------------
Byte *__fastcall ReadMem(DWord Sz) {
    Byte *Result = CurPos;
    SkipBlock(Sz);
    return Result;
}
//------------------------------------------------------------------------------
// Cardinal
void __fastcall ChkSize(Cardinal Sz) {
    if (static_cast<int>(Sz) < 0) {
        printf("[Error] ChkSize: Negative block size %d\n", static_cast<int>(Sz)); // DCUErrorFmt
    }

    if (ScSt.CurPos + Sz > ScSt.EndPos) {
        printf("[Error] ChkSize: Wrong block size %x\n", Sz); // DCUErrorFmt
    }
}

// int OFFSET = 0;
Byte __fastcall ReadByte() {
    // !!!
    /*OFFSET = CurPos - FMemPtr;
    if (OFFSET >= 0x26CE) { // 9934
        OFFSET = OFFSET;
        printf("Debug: ReadByte offset = %d\n", OFFSET);
    }*/
    ChkSize(1);
    Byte Result = *CurPos;
    CurPos++;
    return Result;

    /*ChkSize(1);
    Result := Byte(Pointer(ScSt.CurPos)^);
    Inc(ScSt.CurPos,1);*/
}
//------------------------------------------------------------------------------
void __fastcall ReadByteIfEQ(Byte V) {
    Byte b = *CurPos;
    if (b != V) return;
    CurPos++;
}
//------------------------------------------------------------------------------
/**
 * From DCU_In.pas
 *
 * const
 *  cS12 = [0,2,4,8,$10,$18,$20,$80,$84,Ord(' '),Ord('!'),Ord('a')];
 *  cS12a = cS12+[1];
 *  cS12b = cS12a+[$28,$38];
 *  cS12c = cS12b+[$42,$22,$9];
 *  cS17 = cS12c+[$47,$4F];
 *  cS20 = cS17+[$60];
 *  cS21 = cS20+[$A1];
 *  cS24 = cS21+[$7,$41];
 *  sSkip:array[0..7]of TByteSet = (cS12,cS12a,cS12b,cS12c,cS17,cS20,cS21,cS24);
 *
 *
 * @param V
 * @return
 */
int __fastcall ReadByteFrom(bool V) {
    Byte b = *CurPos;
    // FVer >= verD2010
    if (V) {
        if (b != 0 && b != 1 && b != 2 && b != 4 && b != 8 && b != 16 && b != 24 && b != 32 && b != 33 && b != 97 && b != 128 && b != 132) return -1;
    } else {
        // FVer < verD2010
        if (b != 0 && b != 2 && b != 4 && b != 8 && b != 16 && b != 24 && b != 32 && b != 33 && b != 97 && b != 128 && b != 132) return -1;
    }
    CurPos++;
    return b;
}
//------------------------------------------------------------------------------
int __fastcall ReadByteFrom(TByteSet S) {
    int Result = *reinterpret_cast<const Byte *>(CurPos);
    if (!S.Contains(Result)) {
        return -1;
    }
    CurPos++;
    return Result;
}
//------------------------------------------------------------------------------
TDCURecTag __fastcall ReadTag() {
    DefStart = CurPos;
    return ReadByte();
    // Result := TDCURecTag(ReadByte);
}
//------------------------------------------------------------------------------
Word __fastcall ReadWord() {
    Word Result = *reinterpret_cast<Word*>(CurPos);
    CurPos +=2;
    return Result;
}
//------------------------------------------------------------------------------
// From DCU_In.pas
DWord __fastcall ReadULong() { // ulong
    ChkSize(4);
    DWord Result = *reinterpret_cast<DWord *>(CurPos);
    CurPos += 4;
    return Result;
}
//------------------------------------------------------------------------------
ShortString __fastcall ReadStr() { // was String
    Byte Len = ReadByte();
    ShortString Res = AnsiString(reinterpret_cast<char *>(CurPos), Len);
    CurPos += Len;
    return Res;

    // Result[0] := AnsiChar(ReadByte);
    // ReadBlock(Result[1],Length(Result));
}
//------------------------------------------------------------------------------
PShortName __fastcall ReadShortName() {
    PShortName Result = reinterpret_cast<PShortName>(CurPos);
    SkipBlock(ReadByte());
    return Result;
}
//------------------------------------------------------------------------------
PName __fastcall ReadName() {
    PName Result = reinterpret_cast<PName>(CurPos);
    DWord L = ReadByte();
    if (L == 0xFF && FVer >= verD2009 && FVer < verK1) {
        L = ReadULong();
        printf("Debug: ReadName: 0xFF: L=%d\n", L);
    }
    SkipBlock(L);
    return Result;
}
//------------------------------------------------------------------------------
// Was observed only in drConstAddInfo records of MSIL
AnsiString __fastcall ReadNDXStr() { // was String
    int    L   = ReadUIndex();
    String Res = String(reinterpret_cast<char *>(CurPos), L);
    CurPos += L;
    return Res;
}
//------------------------------------------------------------------------------

/**
 * Appeared in 12 Athens
 * It is very strange, that they don't simply use ReadNDXStr,
 * maybe Len:Val=0 has some special meaning
 * @return
 */
AnsiString __fastcall ReadNDXStrX() { // was String
    int Len = ReadUIndex();

    if (Len > 0) Len--;

    // 1048576
    if (Len < 0 || Len > 0x100000) {
        printf("[Error] ReadNDXStrX: DCUError: Too long NDXX String\n");
        // DCUError('Too long NDXX String');
    }

    AnsiString Res = String(reinterpret_cast<char *>(CurPos), Len); // was String
    CurPos += Len;

    // SetLength(Result,L);
    // ReadBlock(Result[1],L);

    return Res;
}
//------------------------------------------------------------------------------

/**
 * Was observed only in drConstAddInfo records of MSIL
 * Alternative to ReadNDXStr, allows not to read the value
 * @return
 */
TMemStrRef *__fastcall ReadNDXStrRef() {
    int L = ReadUIndex();
    // 1048576
    if (L < 0 && L > 0x100000) printf("[Error] ReadNDXStrRef: DCUError: Too long NDX String\n");
    TMemStrRef *Result = new TMemStrRef(reinterpret_cast<const char *>(ReadMem(L)), L);
    return Result;
}
//------------------------------------------------------------------------------
typedef struct {
    Byte B;
    int  L; // LongInt
} TR4;

// LongInt
int __fastcall ReadUIndex() {
    int    Result;
    Byte   B[4]; // array[0..4] of byte;
    Word  *W  = reinterpret_cast<Word *>(B);
    DWord *L  = reinterpret_cast<DWord *>(B);
    TR4   *R4 = reinterpret_cast<TR4 *>(B);

    // printf("Debug: ReadUIndex: CurPos=%x\n", CurPos);

    NDXHi = 0;
    B[0] = ReadByte();
    if ((B[0] & 1) == 0)
        Result = static_cast<DWord>(B[0] >> 1);
    else {
        B[1] = ReadByte();
        if ((B[0] & 2) == 0)
            Result = static_cast<DWord>(*W >> 2);
        else {
            B[2] = ReadByte();
            B[3] = 0;
            if ((B[0] & 4) == 0)
                Result = *L >> 3;
            else {
                B[3] = ReadByte();
                B[4] = 0;
                if ((B[0] & 8) == 0)
                    Result = *L >> 4;
                else {
                    B[4] = ReadByte();
                    Result = static_cast<DWord>(R4->L);
                    if (FVer > verD3 && ((B[0] & 0xF0) != 0))
                        NDXHi = ReadULong();
                }
            }
        }
    }
    return Result;
}
//------------------------------------------------------------------------------
typedef struct {
    Word  W;
    short i;
} TRL;

int __fastcall ReadIndex() {
    int    Result;
    Byte   B[8];
    char  *SB = reinterpret_cast<char *>(B);
    short *W  = reinterpret_cast<short *>(B);
    int   *L  = reinterpret_cast<int *>(B);
    TR4   *R4 = reinterpret_cast<TR4 *>(B);
    TRL   *RL = reinterpret_cast<TRL *>(B);

    B[0] = ReadByte();
    if ((B[0] & 1) == 0) {
        Result = *SB;
        __asm   sar [Result], 1
    } else {
        B[1] = ReadByte();
        if ((B[0] & 2) == 0) {
            Result = *W;
            __asm   sar [Result], 2
        } else {
            B[2] = ReadByte();
            B[3] = 0;
            if ((B[0] & 4) == 0) {
                RL->i = static_cast<char>(B[2]);
                Result = *L;
                __asm   sar [Result], 3
            } else {
                B[3] = ReadByte();
                if ((B[0] & 8) == 0) {
                    Result = *L;
                    __asm   sar [Result], 3
                } else {
                    B[4] = ReadByte();
                    Result = R4->L;
                    if (FVer > 3 && ((B[0] & 0xF0) != 0)) {
                        NDXHi = ReadULong();
                        return Result;
                    }
                }
            }
        }
    }

    if (Result < 0)
        NDXHi = -1;
    else
        NDXHi = 0;

    return Result;
}
//------------------------------------------------------------------------------
void __fastcall ReadIndex64(PInt64Rec Res) {
    Res->Lo = ReadIndex();
    Res->Hi = NDXHi;
}
//------------------------------------------------------------------------------
void __fastcall ReadUIndex64(PInt64Rec Res) {
    Res->Lo = ReadUIndex();
    Res->Hi = NDXHi;
}
//------------------------------------------------------------------------------
AnsiString __fastcall NDXToStr(int NDXLo) { // was String
    char buf[256];

    if (!NDXHi)
        sprintf(buf, "$%lX", NDXLo);
    else if (NDXHi == -1)
        sprintf(buf, "-$%lX", -NDXLo);
    else if (NDXHi < 0)
        sprintf(buf, "-$%lX%08lX", -NDXHi - 1, -NDXLo);
    else
        sprintf(buf, "$%lX%08lX", NDXHi, NDXLo);

    return String(buf);
}
//------------------------------------------------------------------------------
#ifdef LINUX
const char AlterSep = '\\';
#else
const char AlterSep = '/';
#endif
String __fastcall ExtractFileNameAnySep(String FN) {
    String Result = ExtractFileName(FN);
    char  *CP     = StrRScan(AnsiString(Result).c_str(), AlterSep);
    if (!CP)
        return Result;
    else
        return StrPas(CP + 1);
}
//------------------------------------------------------------------------------

/**
 * Extract file name for packaged or normal files
 *
 * @param FN
 * @return
 */
String ExtractFileNamePkg(const String FN) {
    String Result = ExtractFileName(FN);
    char* CP = StrScan(AnsiString(Result).c_str(), AlterSep);
    if (CP) {
        Result = StrPas(CP + 1);
    }
    return Result;
}
//------------------------------------------------------------------------------
TDCURecTag __fastcall FixTag(TDCURecTag recTag) {
    TDCURecTag Result = recTag;
    if (FVer >= verD2006 && FVer < verK1) {
        // In D10 some codes were changed, we'll try to move them back
        if (Result >= 0x2D && Result <= 0x36) {
            Result--;
            if (Result < 0x2D)
                Result = 0x36; // This code could be wrong, but the overloaded value of $2D should be moved somewhere
        }
    }
    return Result;
}
//------------------------------------------------------------------------------

/**
 * TUnit.RegisterEmbeddedTypes
 *
 * In Delphi XE DCU local data type`s declarations are placed out of the list of
 * procedure local declarations, and several types from different procedures could
 * be placed into one common list. Here we try to find the place where the type should be
 *
 * @param Embedded
 * @param Depth
 */
void __fastcall RegisterEmbeddedTypes(TDCURec *Embedded, int Depth) {
    // PDCURec *DP = reinterpret_cast<PDCURec *>(Embedded);
    TDCURec **DP = &Embedded;

    while (true) {
        TDCURec *D = *DP;
        if (D == nullptr) return;
        // The effect was noticed only for types
        if (D->InheritsFrom(__classid(TTypeDecl))) {
            PTypeDecl TD = static_cast<PTypeDecl>(D);
            if (FEmbeddedTypes == 0) FEmbeddedTypes = new TList;
            ChkListSize(FEmbeddedTypes, TD->hDef);
            PEmbeddedTypeInf TI = static_cast<PEmbeddedTypeInf>(FEmbeddedTypes->Items[TD->hDef - 1]);
            if (TI == nullptr) {
                TI        = new TEmbeddedTypeInf;
                TI->TD    = nullptr;
                TI->Depth = 0;
                FEmbeddedTypes->Items[TD->hDef - 1] = TI;
            } else if (TI->TD->hDecl > TD->hDecl) {
                TI->TD = nullptr;
            }
            if (TI->TD == nullptr) {
                TI->TD    = TD;
                TI->Depth = Depth;
            }
            *DP = D->Next;
        } else
            DP = &(D->Next);
    }
}
//------------------------------------------------------------------------------
typedef struct {
    TList   *EmbL;
    TList   *EmbeddedTypes;
} TBindEmbeddedTypeInf, *PBindEmbeddedTypeInf;

/**
 * TUnit.BindEmbeddedType;
 *
 * @param UseRec // change to TDCURec?
 * @param hDT // TDefNDX
 * @param IP Pointer
 */
void __fastcall BindEmbeddedType(PDCURec UseRec, int hDT, DWord* IP) {
    PBindEmbeddedTypeInf betf = reinterpret_cast<PBindEmbeddedTypeInf>(IP);

    if (hDT <= 0 || hDT > betf->EmbeddedTypes->Count) return;
    PEmbeddedTypeInf TI = (PEmbeddedTypeInf) betf->EmbeddedTypes->Items[hDT - 1];
    if (TI == nullptr) return;
    if (TI->Depth > betf->EmbL->Count) return;
    betf->EmbeddedTypes->Items[hDT - 1] = 0;
    PProcDecl PD = (PProcDecl) betf->EmbL->Items[TI->Depth - 1];
    PTypeDecl TD = TI->TD;
    // TD->Next = PD->Locals;
    // PD->Locals = TD;
    PD->AddLocal(TD);
    delete TI;
    TD->EnumUsedTypes(BindEmbeddedType, IP);
}
//------------------------------------------------------------------------------
void __fastcall EnumUsedTypeList(PDCURec L, TTypeUseAction Action, DWord *IP) {
    while (L != 0) {
        L->EnumUsedTypes(Action, IP);
        L = L->Next;
    }
}
//------------------------------------------------------------------------------
TBindEmbeddedTypeInf BindEmbeddedTypeInf;

void __fastcall CheckProcedures(PDCURec D) {
    while (D != 0) {
        if (D->InheritsFrom(__classid(TProcDecl))) {
            BindEmbeddedTypeInf.EmbL->Add(D);
            EnumUsedTypeList(static_cast<PProcDecl>(D)->Locals, BindEmbeddedType, reinterpret_cast<DWord *>(&BindEmbeddedTypeInf));
            EnumUsedTypeList(static_cast<PProcDecl>(D)->Embedded, BindEmbeddedType, reinterpret_cast<DWord *>(&BindEmbeddedTypeInf));
            CheckProcedures(static_cast<PProcDecl>(D)->Embedded);
            BindEmbeddedTypeInf.EmbL->Count = BindEmbeddedTypeInf.EmbL->Count - 1;
        }
        D = D->Next;
    }
}

/**
 * TUnit.BindEmbeddedTypes;
 */
void __fastcall BindEmbeddedTypes() {
    if (FEmbeddedTypes == nullptr) return;
    BindEmbeddedTypeInf.EmbL          = new TList;
    BindEmbeddedTypeInf.EmbeddedTypes = FEmbeddedTypes;
    CheckProcedures(FDecls);
    delete BindEmbeddedTypeInf.EmbL;
    for (int i = FEmbeddedTypes->Count - 1; i >= 0; i--) {
        PEmbeddedTypeInf TI = static_cast<PEmbeddedTypeInf>(FEmbeddedTypes->Items[i]);
        if (TI == nullptr) continue;
        TI->TD->Next = FDecls;
        FDecls       = TI->TD;
        TI->TD->ListAppend(FDecls);
        delete TI;
    }
    delete FEmbeddedTypes;
    FEmbeddedTypes = 0;
}
//------------------------------------------------------------------------------
Byte __fastcall ReadCallKind() {
    Byte Result = pcRegister;
    if (Tag >= arCDecl && Tag <= arSafeCall) {
        Result = Tag - arCDecl + 1;
        Tag    = ReadTag();
    }
    return Result;
}
//------------------------------------------------------------------------------
/**
 * From DCURecs.pas
 *
 * @param PITbl
 * @return
 */
int __fastcall ReadClassInterfaces(PPNDXTbl PITbl) {
    // printf("Debug: ReadClassInterfaces: CurPos = %x, FMemSize = %x\n", CurPos, FMemSize);
    int Result = ReadIndex();
    if (Result <= 0) return Result;
    PNDXTbl ITbl = nullptr;
    if (PITbl) {
        ITbl   = new TNDXTbl[Result * 2 * sizeof(TNDX)];
        *PITbl = ITbl;
    }
    // printf("Debug: ReadClassInterfaces: Result = %d\n", Result);
    for (int i = 0; i < Result; i++) {
        TNDX MatchCnt;
        int X1;
        int hIntf = ReadUIndex();
        if (IsMSIL && FVer >= verD2006 && FVer < verK1) {
            X1 = ReadUIndex();
            MatchCnt = ReadUIndex();
        }
        int MCnt = ReadUIndex();
        if (ITbl) {
            *ITbl[2 * i]     = hIntf;
            *ITbl[2 * i + 1] = MCnt;
        }
        if (IsMSIL) {
            for (int j = 1; j <= MCnt; j++) {
                int N = ReadUIndex();
                int hMember = ReadUIndex();
            }
        } else if (FVer >= verD2006 && FVer < verK1) {
            X1 = ReadUIndex();
            MatchCnt = ReadUIndex();
            if (FVer >= verD2010) {
                TNDX X3 = ReadUIndex();
                TNDX X4 = ReadUIndex();
                for (int j = 1; j <= MatchCnt; j++) {
                    // printf("Debug: ReadClassInterfaces: i=%d, Max=%d, MatchCnt=%d, j=%d\n",i, Result, MatchCnt, j);
                    // printf("Debug: ReadClassInterfaces: MatchCnt=%d, j=%d CurPos=%x\n", MatchCnt, j, CurPos);
                    Byte B = ReadByte();
                    PName MName = ReadName();
                    // printf("Debug: ReadClassInterfaces: MatchCnt=%d, j=%d CurPos=%x, Name: %s\n", MatchCnt, j, CurPos, MName->GetStr().c_str());
                    int N = ReadUIndex();
                    int hMember = ReadUIndex();
                    // printf("Debug: ReadClassInterfaces: MatchCnt=%d, j=%d, B=%d, N=%d, hMember=%d, CurPos=%x, Name: %s\n", MatchCnt, j, B, N, hMember, CurPos, MName->GetStr().c_str());
                    // ReadUIndex(); // +4
                    // ReadUIndex(); // +8
                    // ReadByte();   // =1
                    // ReadName();
                }
                if (FVer >= verD12) {
                    AnsiString AName = ReadNDXStrX();
                }
            }
        }
    }
    return Result;
}
//------------------------------------------------------------------------------

String __fastcall GetVersionStr() {
    static const String verStrDelphi[MaxDelphiVer + 1] = {
        L"Error", L"Error 1", L"2", L"3", L"4", L"5", L"6", L"7", L"8", L"2005",
        L"2006", L"?2007", L"2009", L"Error 13", L"2010", L"XE", L"XE2", L"XE3",
        L"XE4", L"XE5", L"XE6", L"XE7", L"XE8", L"10 Seattle", L"10.1 Berlin",
        L"10.2 Tokyo", L"10.3 Rio", L"10.4 Sydney", L"11 Alexandria", L"12 Athens",
        L"13 Florence"
    };

    static const String platfStr[static_cast<int>(TDCUPlatform::Linux64) + 1] = {
        L"Win32", L"Win64", L"Osx32", L"Osx64", L"OsxArm64",
        L"iOSEmulator", L"iOSSimArm64", L"iOSDevice", L"iOSDevice64",
        L"Android", L"Android64", L"Linux64"
    };

    String Result;
    if (FVer < verK1) {
        Result = "Delphi " + verStrDelphi[FVer];
        if (FVer >= verDXE2) {
            Result = Format("%s (%s)",ARRAYOFCONST((Result, platfStr[static_cast<int>(FPlatform)])));
        }
    } else {
        Result = Format("Kylix %d", ARRAYOFCONST((FVer - verK1 + 1)));
    }

    return Result;
}
//------------------------------------------------------------------------------

/**
 * TUnit.ShowSourceFiles();
 */
static void __fastcall ShowSourceFiles() {
    ModuleInfo->Name = UnitName;
#ifdef SHOW
    OutLog2("Unit %s\n", UnitName.c_str());
    if (FVer > verD2) {
        OutLog2("Flags: %lX\n", Flags);
        if (FVer > verD3) OutLog2("Priority %lX\n", UnitPrior);
    }

    OutLog2("Compiled by %s\n", AnsiString(GetVersionStr()).c_str());

    OutLog1("Source files:\n");
    for (int n = 0; n < FSrcFiles->Count; n++) {
        PSrcFileRec SFR = static_cast<PSrcFileRec>(FSrcFiles->Items[n]);
        if (!SFR) break;
        Byte T = SFR->Def->Tag;

        switch (T) {
            case drSrc: OutLog1("src"); break;
            case drRes: OutLog1("res"); break; // $R
            case drObj: OutLog1("obj"); break; // $L
            case drAsm: OutLog1("asm"); break;
            case drAssemblyInfo: OutLog1("$Assembly"); break;
            case drAssemblySrc: OutLog1("$Assembly8"); break;
            default: break;
        }
        if (T == drAssemblyInfo) {
            OutLog2("%s\n", ReadNDXStr().c_str());
        } else {
            // was: OutLog2("%s\n", AnsiString(SFR->Def->Name.Name, SFR->Def->Name.Len).c_str());
            // new:
            OutLog2("%s\n", SFR->Def->Name.GetStr().c_str());
        }

        // if (integer(SFR^.FT)<>-1)and(integer(SFR^.FT)<>0) then
       //     PutSFmt(' (%s)',[FileDateToStr(SFR^.FT)]);

        // new: OutLog2(" %s\n", SFR->Def->Name->GetStr().c_str());
        // OutLog2(" %s\n", AnsiString(SFR->Def->Name.Name, SFR->Def->Name.Len).c_str());
    }
#endif
}
//------------------------------------------------------------------------------
static void __fastcall ReadSourceFiles() {
    PSrcFileRec SFRMain = nullptr;
    FSrcFiles = new TList;

    while (Tag == drSrc || Tag == drRes || Tag == drObj || Tag == drAsm ||
           (FVer >= verD2010 && FVer < verK1 && Tag == drUnitInlineSrc) ||
           (FVer >= verD2005 && FVer < verK1 && IsMSIL && Tag == drAssemblyInfo) ||
           (IsMSIL && Tag == drAssemblySrc)) {
        PSrcFileRec SFR = new TSrcFileRec;
        SFR->Def = reinterpret_cast<PNameDef>(DefStart);
        // SFR->Lines = nullptr;

        if (Tag == drAssemblyInfo) {
            ReadNDXStr();
            SFR->FT = 0;
            SFR->Ndx = 0;
        } else {
            ReadName();
            SFR->FT = ReadULong(); // FileTime
            int F = ReadUIndex();
            if (!F) SFRMain = SFR;
            SFR->Ndx = F;
            if (IsMSIL && Tag != drRes && Tag != drAssemblySrc)
                ShortString SrcFName = ReadStr(); // Ignored by now, because it's always empty

            FSrcFiles->Add(static_cast<void *>(SFR)); // SFR->Next
        }
        Tag = ReadTag();
    }

    if (!FSrcFiles->Count) {
        if (!FromPackage) printf("[Error] ReadSourceFiles: No source files found\n"); // DCUError('No source files');
        return;
    }

    if (!SFRMain)
        SFRMain = (PSrcFileRec)FSrcFiles->Items[0];

    if (!SFRMain || (SFRMain->Def->Tag == drAssemblyInfo || SFRMain->Def->Tag == drAssemblySrc)) {
        UnitName = ChangeFileExt(ExtractFileNamePkg(FFName),"");
    } else {
        UnitName = ExtractFileNameAnySep(SFRMain->Def->Name.GetStr());
        char *CP = StrRScan(UnitName.c_str(), '.');
        UnitName.SetLength(CP - UnitName.c_str());
    }
}
//------------------------------------------------------------------------------
static void __fastcall ReadUses(TDCURecTag TagRq) {
    int       ndx;
    DWord     RTTISz;
    int       L;
    PName     ImpN;
    TBaseDef *TR, *AR;

    int hUses   = 0;
    int ImpBase = 0;

    while (Tag == TagRq) {
        PName       UseName = ReadName();
        PUnitImpRec pUnit   = new TUnitImpRec; // U
        memset(pUnit, 0, sizeof(TUnitImpRec));
        pUnit->Name = UseName;
        char Ch = '?';

        switch (TagRq) {
            case drUnit1: // 0x65
                Ch       = 'U';
                pUnit->Flags = TUnitImpFlags::Impl; // ufImpl;
                break;
            case drDLL: // 0x68
                Ch       = 'D';
                pUnit->Flags = TUnitImpFlags::DLL; // ufDLL;
                break;
            case drDLLInfo1: // 0xB3
                Ch       = 'E';
                pUnit->Flags = TUnitImpFlags::DLL1;// ufDLL1;
                break;
        }

        int hUnit = FUnitImp->Count;
        FUnitImp->Add(static_cast<void *>(pUnit));
        int hPack = 0;

        if (TagRq != drDLL && TagRq != drDLLInfo1 && FVer >= verD8 && FVer < verK1)
            hPack = ReadUIndex();
            
        if (FVer >= verD2006 && FVer < verK1)
            L = ReadUIndex();
        else
            L = ReadULong();

        if ((FVer == verD7 && FVer < verK1) || (FVer >= verD8 && FVer < verK1 && TagRq == drDLL) || (TagRq == drDLLInfo1))
            int L1 = ReadULong();

        if (FVer >= verD2009 && FVer < verK1)
            int L2 = ReadUIndex();

        TBaseDef *DeclEnd = pUnit->Decls;
        int hImp = 0;
        TUnitImpDef *UIR = new TUnitImpDef(Ch, UseName, L, nullptr, hUnit); // Unit reference
        pUnit->Ref      = UIR;
        int ImpBase0    = ImpBase;
        ImpBase         = AddAddrDef(UIR);

        if (hPack > 0 && FVer < verD2009) // or may be MSIL only
            RefAddrDef(hPack); // Reserve index for unit package number

        while (true) {
            Tag = ReadTag();
            if (Tag == drImpType || Tag == drImpTypeDef) {
                if (TagRq != drDLL && TagRq != drDLLInfo1) { // 0x68 && 0xB3
                    Ch   = 'T';
                    ImpN = ReadName();
                    if (Tag == drImpTypeDef) RTTISz = ReadUIndex();
                    L = ReadULong();
                    if (Tag == drImpTypeDef)
                        TR = new TImpTypeDefRec(ImpN, L, RTTISz, NULL, hUnit);
                    else
                        TR = new TImpDef('T', ImpN, L, NULL, hUnit);

                    FTypes->Add(static_cast<void *>(TR));
                    TR->hDecl   = AddAddrDef(TR);
                    ndx         = FTypes->Count;
                    FTypeDefCnt = ndx;
                }
            } else if (Tag == drImpVal) {
                Ch   = 'A';
                ImpN = ReadName();
                L    = ReadULong();
                if (TagRq != drDLL && TagRq != drDLLInfo1)
                    AR = new TImpDef('A', ImpN, L, NULL, hUnit);
                else
                    AR = new TDLLImpRec(ImpN, L, NULL, hUnit);
                ndx = AddAddrDef(AR);
                TR  = AR;
            } else if (Tag == drStop2) {
                // Imports drConstAddInfo may be for the prev. drImpVal always
                L = -1;
                if (FVer >= verD8 && FVer < verK1) L = ReadULong(); // ==IP for the imported drConstAddInfo
                continue;
            } else if (Tag == drConstAddInfo) {
                if (!(FVer >= verD11 && FVer < verK1)) { // It may be used now with Tag:08 to store defines
                    if (!IsMSIL) break;
                    if (hImp) printf("[Error] ConstAddInfo encountered for %s in subrecord #%d\n", UseName, hImp); // DCUErrorFmt
                }
                int ImpReBase = ReadConstAddInfo(nullptr); // Just skip it by now
                continue;
            } else {
                break;
            }
            DeclEnd = TR;
            DeclEnd = static_cast<TBaseDef *>(TR->Next);

            hImp++;
        }

        if (Tag != drStop1) printf("[Error] Unexpected tag: %lX\n", Tag); // DCUErrorFmt

        hUses++;
        Tag = ReadTag();
        // 0x9E
        if (Tag == drProcAddInfo) {
            // The only tag by now, which was observed between imports
            if (FVer < verD7 || FVer >= verK1) break;
            hImp = ReadIndex();
            SetProcAddInfo(hImp);
            Tag = ReadTag();
        }
    }
}
//------------------------------------------------------------------------------

/**
 * TUnit.ShowUses(const PfxS: AnsiString; FRq: TUnitImpFlags)
 *
 * @param PfxS
 * @param FRq
 */
void __fastcall ShowUses(AnsiString PfxS, TUnitImpFlags FRq) {
    // if (FUnitImp->Count == 0) return;
    int Cnt = 0;
    for (int i = 0; i < FUnitImp->Count; i++) {
        PUnitImpRec U = (PUnitImpRec) FUnitImp->Items[i];
        if (FRq != U->Flags) continue;
        // new:
        String name = U->Name->GetStr();
        // was: String name = String(U->Name->Name, U->Name->Len);
        if (ModuleInfo->UsesList->IndexOf(name) == -1)
            ModuleInfo->UsesList->Add(name);
        if (Cnt > 0) {
            OutLog1(",");
        } else {
            OutLog2("%s ", PfxS.c_str());
        }
        OutLog2("%s", AnsiString(name).c_str());
        Cnt++;
    }
    if (Cnt > 0) OutLog1(";");
    OutLog1("\n");
}
//------------------------------------------------------------------------------

/**
 * To the end of list - for TCopyDecl
 * @param ND
 * @return FAddrs->Count
 */
int AppendAddrDef(TDCURec *ND) {
    FAddrs->Add(ND);
    return FAddrs->Count;
}
//------------------------------------------------------------------------------

/**
 * TUnit.SetDeclMem
 *
 * @param hDef
 * @param Ofs
 * @param Sz
 */
void __fastcall SetDeclMem(int hDef, DWord Ofs, DWord Sz) {
    if (hDef <= 0 || hDef > FAddrs->Count) {
        printf("[Error] DCUError: Undefined Fixup Declaration: #%x\n", hDef); // DCUErrorFmt
        return;
    }
    TDCURec *D    = static_cast<TDCURec *>(FAddrs->Items[hDef - 1]);
    DWord    Base = 0;

    while (D) {
        /*if (D->InheritsFrom(__classid(TProcDecl))) {
            ((TProcDecl *) D)->AddrBase = Base;
        }*/
        if (TProcDecl *PD = dynamic_cast<TProcDecl*>(D)) {
            PD->AddrBase = Base;
        }
        DWord Rest = D->SetMem(Ofs + Base, Sz - Base);
        if (static_cast<int>(Rest) <= 0) break;
        Base = Sz - Rest;
        D    = D->Next; // Next declaration - should be procedure
    }
}
//------------------------------------------------------------------------------
/**
 * TUnit.LoadFixups
 */
void LoadFixups() {
    if (FFixupTbl){
        printf("[Error] DCUError: 2nd fixup\n"); // DCUError
        // raise error
    }

    FFixupCnt = ReadUIndex();
    FFixupTbl = new TFixupRec[FFixupCnt]; // AllocMem(FFixupCnt*sizeof(TFixupRec));

    memset(FFixupTbl, 0, FFixupCnt * sizeof(TFixupRec));

    // TFixupRec *FP = FFixupTbl;
    PFixupRec FP = FFixupTbl;
    DWord CurOfs = 0;
    Byte  B1;

    for (int i = 0; i < FFixupCnt; i++) {
        DWord dOfs = ReadUIndex();
        CurOfs += dOfs;
        if (NDXHi != 0 || CurOfs > FDataBlSize) {
            printf("[Error] LoadFixups: DCUError: Fixup offset $%x Block size = $%x\n", CurOfs, FDataBlSize); // DCUErrorFmt
        }
        B1       = ReadByte();
        FP->OfsF = (CurOfs & FixOfsMask) | (B1 << 24);
        FP->Ndx  = ReadUIndex();
        FP++;
    }

    // After loading fixups set the memory sizes of CBlock parts
    CurOfs = 0;
    FP = FFixupTbl;
    int hPrevDecl = 0;
    DWord PrevDeclOfs = 0;

    for (int i = 0; i < FFixupCnt; i++) {
        CurOfs = (FP->OfsF & FixOfsMask);
        // B1 = (*reinterpret_cast<const TByte4*>(&FP->OfsF))[3];
        // B1 = reinterpret_cast<const Byte *>(&FP->OfsF)[3];
        B1 = (&FP->OfsF)[3];
        // !!!
        // if (B1 != 12 && B1 != 1 && B1 != 2 && B1 != 3 && B1 != 5 && B1 != 13)
        // B1 = B1;
        if (B1 == fxStart || B1 == fxEnd) {
            if (hPrevDecl > 0) {
                SetDeclMem(hPrevDecl, PrevDeclOfs, CurOfs - PrevDeclOfs);
            }
            hPrevDecl   = FP->Ndx;
            PrevDeclOfs = CurOfs;
            FDataBlOfs  = CurOfs;
        }
        FP++;
    }
}
//------------------------------------------------------------------------------

/**
 * TUnit.LoadCodeLines
 */
void LoadCodeLines() {
    if (FCodeLineTbl) {
        printf("[Error] 2nd Code Lines table\n"); // DCUError
    }

    FCodeLineCnt = ReadUIndex();
    FCodeLineTbl = static_cast<PCodeLineTbl>(AllocMem(FCodeLineCnt * sizeof(TCodeLineRec)));

    int   CurL         = 0;
    DWord CurOfs       = 0;
    bool error = false;
    PCodeLineRec CR = *FCodeLineTbl;

    for (int i = 0; i < FCodeLineCnt; i++) {
        int dL = ReadIndex();
        // The file lib\win64\debug\Data.Bind.ObjectScope.dcu of D_12 contains some
        // strange records with (dL:32465; dOfs:0xFFFB).
        // In fact the max line number in drLinNum is 22980 for this file, so this
        // value of dL is very strange. After several records of this kind we got
        // an error of offset outside CBlock.
        // Because I was unable to find any other files with this kind of records,
        // I believe by now that it is a bug of template processing in D_12 compiler
        // and stopped trying to fix it somehow
        DWord dOfs = ReadUIndex();
        if (error) continue;
        CurOfs += dOfs;
        if (dL == 0) continue; // it is an aux record to keep dOfs<=0xFFFF
        CurL += dL;
        if (!FromPackage && (NDXHi != 0 || CurOfs > FDataBlSize)) {
            // The only fix for the 0xFFFB bug is here: I have replaced Error by Warning
            printf("[Error] Code line offset[%d] $%x>Block size = S%x\n", i, CurOfs, FDataBlSize);
            //  DCUWarningFmt{DCUErrorFmt}('Code line offset[%d] $%x>Block size = S%x',[i,CurOfs,FDataBlSize]);
            error = true; // Should read the table up to the end
            continue;
        }
        // in the file debug\MidasLib.dcu of D2009 (which was compiled from a lot of C
        // and H files) the records 17291..82826 (exactly $10000 recs)
        // contain dL=0, dOfs=$FFFF. The same file in D2010 doesn't contain such records.
        // So i believe it's a bug of D2009 and won't fix it
        CR->ofs = CurOfs;
        CR->L = CurL;
        CR++;
    }
    // Some aux records could be skipped
    FCodeLineCnt = (reinterpret_cast<TIncPtr>(CR) - reinterpret_cast<TIncPtr>(FCodeLineTbl)) / sizeof(TCodeLineRec);
    // FCodeLineCnt = (TIncPtr(CR)-TIncPtr(FCodeLineTbl)) div SizeOf(TCodeLineRec);
}

/*
 *procedure TUnit.LoadCodeLines;
var
  i,CurL,dL,dOfs: integer;
  CR: PCodeLineRec;
  CurOfs: Cardinal;
  Err: Boolean;
begin
  if FCodeLineTbl<>Nil then
    DCUError('2nd Code Lines table');
  FCodeLineCnt := ReadUIndex;
  FCodeLineTbl := AllocMem(FCodeLineCnt*SizeOf(TCodeLineRec));
  CurL := 0;
  CurOfs := 0;
  Err := false;
  CR := Pointer(FCodeLineTbl);
  for i:=0 to FCodeLineCnt-1 do begin
    dL := ReadIndex;
    {It doesn't help
    if SignedDOfs then begin
      dOfs := ReadIndex;  //It is possible, that it always was signed,
        //but only in a unit of D 12 win64 debug (Data.Bind.ObjectScope.dcu) it was observed 1st
      if (dOfs<0)and(NDXHi=-1) then
        NDXHi := 0;
     end
    else}
    //The file lib\win64\debug\Data.Bind.ObjectScope.dcu of D_12 contains some
    //strange records with (dL:32465; dOfs:0xFFFB).
    //In fact the max line number in drLinNum is 22980 for this file, so this
    //value of dL is very strange. After several records of this kind we got
    //an error of offset outside CBlock.
    //Because I was unable to find any other files with this kind of records,
    //I believe by now that it is a bug of template processing in D_12 compiler
    //and stopped trying to fix it somehow
    dOfs := ReadUIndex;
    if Err then
      continue;
    Inc(CurOfs,dOfs);
    if dL=0 then
      continue; //it is an aux record to keep dOfs<=0xFFFF
    Inc(CurL,dL);
    if not FromPackage and((NDXHi<>0)or(CurOfs>FDataBlSize)) then begin
     //The only fix for the 0xFFFB bug is here: I have replaced Error buy Warning
      DCUWarningFmt{DCUErrorFmt}('Code line offset[%d] $%x>Block size = S%x',[i,CurOfs,FDataBlSize]);
      Err := true; //Should read the table up to the end
      continue;
    end;
    {in the file debug\MidasLib.dcu of D2009 (which was compiled from a lot of C
     and H files) the records 17291..82826 (exactly $10000 recs)
     contain dL=0, dOfs=$FFFF. The same file in D2010 doesn't contain such records.
     So i believe it's a bug of D2009 and won't fix it}
    CR^.Ofs := CurOfs;
    CR^.L := CurL;
    Inc(CR);
  end ;
  FCodeLineCnt := (TIncPtr(CR)-TIncPtr(FCodeLineTbl))div SizeOf(TCodeLineRec); //Some aux records could be skipped
end ;
 */
//------------------------------------------------------------------------------
void LoadLineRanges() {
    DWord FLineRangeCnt = ReadUIndex();

    int Num = 0;

    for (int i = 0; i < FLineRangeCnt; i++) {
        DWord Line0 = ReadUIndex();
        DWord LineNum = ReadUIndex();
        Num += LineNum;
        int hFile = ReadUIndex();
    }
}
//------------------------------------------------------------------------------
/**
 * TUnit.LoadStrucScope
 */
void LoadStrucScope() {
    int Cnt = ReadUIndex();
    int N = 5;
    if (FVer >= verD10_3 && FVer < verK1) N++; // Some field was added
    for (int i = 0; i < Cnt * N; i++) ReadUIndex(); // hType,hVar,Ofs,LnStart,LnCnt
}
//------------------------------------------------------------------------------
/**
 * TUnit.LoadSymbolInfo
 */
void LoadSymbolInfo() {
    int Cnt      = ReadUIndex();
    int NPrimary = ReadUIndex();
    for (int i = 0; i < Cnt; i++) {
        int hSym    = ReadUIndex();
        int hMember = ReadUIndex(); // for symbols - type members, else - 0
        int Sz      = ReadUIndex();
        int hDef    = ReadUIndex(); // index of symbol definition in the L array
        for (int j = 0; j < Sz; j++) ReadUIndex();
    }
}
//------------------------------------------------------------------------------
/**
 * TUnit.LoadLocVarTbl
 */
void LoadLocVarTbl() {
    if (FLocVarTbl)
        printf("[Error] LoadLocVarTbl: FLocVarTbl already allocated (2nd Local Vars table)\n"); // DCUError

    FLocVarCnt = ReadUIndex();
    FLocVarSize = FLocVarCnt;

    if (FPlatform != dcuplWin64) {
        FLocVarTbl = new TLocVarRec[FLocVarCnt];
        memset(FLocVarTbl, 0, FLocVarCnt * sizeof(FLocVarCnt));
        PLocVarRec LR = FLocVarTbl;

        for (int i = 0; i < FLocVarCnt; i++) {
            LR->sym   = ReadUIndex();
            LR->ofs   = ReadUIndex();
            LR->frame = ReadIndex();
            LR++;
        }
    } else {
        int F;
        // They write additional record in 64-bit mode for each procedure without fixing FLocVarCnt
        FLocVarSize = (FLocVarCnt * 3) / 2;
        FLocVarTbl = new TLocVarRec[FLocVarCnt];
        memset(FLocVarTbl, 0, FLocVarCnt * sizeof(FLocVarCnt));
        PLocVarRec LR = FLocVarTbl;
        int ProcRec = 0;
        int i = 0;
        while (i < FLocVarCnt) {
            int Sym = ReadUIndex();
            if (ProcRec == 0) {
                if (Sym != 0) {
                    TDCURec *D = GetAddrDef(Sym);
                    if (TProcDecl *PD = dynamic_cast<TProcDecl*>(D)) {
                        ProcRec = 3;
                        i--;
                    }
                }

                LR->sym = Sym;
                LR->ofs = ReadUIndex();

                if (ProcRec > 1)
                    F = ReadUIndex();
                else
                    F = ReadIndex();

                LR->frame = F;
                LR++;
                i++;

                if (ProcRec > 0)
                    ProcRec--;
            }
        }

        int Sz = (reinterpret_cast<TIncPtr>(LR) - reinterpret_cast<TIncPtr>(FLocVarTbl)) / sizeof(TLocVarRec);
        if (FLocVarSize != Sz) {
            ReallocMemory(FLocVarTbl, FLocVarSize * sizeof(TLocVarRec));
            FLocVarSize = Sz;
        }
    }

    /*FLocVarTbl = new TLocVarRec[FLocVarCnt];
    memset(FLocVarTbl, 0, FLocVarCnt * sizeof(FLocVarCnt));
    PLocVarRec LR = FLocVarTbl;

    for (int i = 0; i < FLocVarCnt; i++) {
        LR->sym   = ReadUIndex();
        LR->ofs   = ReadUIndex();
        LR->frame = ReadIndex();
        LR++;
    }*/
}
//------------------------------------------------------------------------------
void ClearAddrDef(TNameDecl *ND) {
    if (FLoaded) return;
    if (!FAddrs) return;

    int hDecl = ND->hDecl - 1;
    if (hDecl < 0 || hDecl >= FAddrs->Count) return;

    if (FAddrs->Items[hDecl] == ND) {
        FAddrs->Items[hDecl] = nullptr;
    }
}
//------------------------------------------------------------------------------

/**
 * This procedure is called from TTypeDef.Destroy, and required when
 * Destroy is called due to errors in Create
 * @param TD
 */
void ClearLastTypeDef(TTypeDef *TD) {
    if (FLoaded || (FTypeDefCnt <= 0)) return;

    if ((TTypeDef *)FTypes->Items[TD->hDT] == TD)
        delete FTypes->Items[TD->hDT];
        // FTypes->Items[TD->hDT] = nullptr;
}
//------------------------------------------------------------------------------
/**
 * TUnit.AddAddrDef
 * @param ND
 * @return
 */
int __fastcall AddAddrDef(TDCURec *ND) {
    if (FhNextAddr > 0) {
        int Result = FhNextAddr;
        if (Result > FAddrs->Count) {
            printf("[Error] ProcAddInfo Value $%x>FAddrs.Count=$%x\n", Result, FAddrs->Count); // DCUErrorFmt
        }

        if (FAddrs->Items[Result - 1]) {
            TDCURec *Rec = static_cast<TDCURec *>(FAddrs->Items[Result - 1]);
            PName NP = Rec->Name;

            if (!NP) {
                NP = &NoName;
                // NP = GetNoName();
            }

            // was: printf("[Error] FAddrs[$%x] already used by %s\n", Result, AnsiString(PName2String(NP)).c_str()); // DCUErrorFmt
            // new:
            printf("[Error] FAddrs[$%x] already used by %s\n", Result, NP->GetStr().c_str()); // DCUErrorFmt
        }

        FAddrs->Items[Result - 1] = static_cast<void *>(ND);
        // if (FVer >= verDXE1 && FVer < verK1)
        //     FhNextAddr = -1;
        // else
        FhNextAddr++;
        return Result;
    }
    FAddrs->Add(static_cast<void *>(ND));
    return FAddrs->Count;
}
//------------------------------------------------------------------------------

/**
 * TUnit.RefAddrDef
 * This procedure is used for addrs, which may be forward references to the objects,
 * which don't yet exist. To fill the empty slot the drProcAddInfo tag is used after
 * creation of the object.
 * @param V
 */
void __fastcall RefAddrDef(int V) {
    if (V > FAddrs->Count) {
        if (V != FAddrs->Count + 1)
            printf("[Error] Unexpected forward hDecl=0x%x<>0x%x\n", V, FAddrs->Count + 1); // DCUErrorFmt
        FAddrs->Add(NULL); // This way it won't interfere with FhNextAddr
    }
}
//------------------------------------------------------------------------------
/**
 * TUnit.GetAddrDef
 * @param hDef
 * @return TDCURec*
 */
TDCURec *__fastcall GetAddrDef(int hDef) {
    if (hDef <= 0 || hDef > FAddrs->Count) return nullptr;
    return reinterpret_cast<TDCURec *>(FAddrs->Items[hDef - 1]);
}
//------------------------------------------------------------------------------
// Moved from DCUClasses
PName __fastcall GetAddrName(int hDef) {
    TDCURec *D = GetAddrDef(hDef);
    if (!D) return &NoName;
    // new: if (!D) return GetNoName();
    return D->Name;
}
//------------------------------------------------------------------------------

/**
 * TUnit.GetAddrStr
 * @param hDef NDX
 * @param ShowNDX
 * @return
 */
AnsiString __fastcall GetAddrStr(int hDef, bool ShowNDX) { // was String
    return GetDCURecStr(GetAddrDef(hDef), hDef);
}
//------------------------------------------------------------------------------

/**
 * TUnit.GetLocalTypeDef
 *
 * @param hDef
 * @return
 */
TTypeDef *__fastcall GetLocalTypeDef(int hDef) {
    // The type should be from this unit
    TBaseDef *D = GetTypeDef(hDef);
    if (D->InheritsFrom(__classid(TTypeDef)))
        return static_cast<TTypeDef *>(D);
    // TImpDef
    return nullptr;
}
//------------------------------------------------------------------------------
TTypeDef *__fastcall GetGlobalTypeDef(int hDef) {
    TBaseDef *D = GetTypeDef(hDef);

    while (true) {
        PName N;
        int hUnit;
        if (!D) return nullptr;
        if (D->InheritsFrom(__classid(TTypeDef))) break; // Found - OK
        if (!D->InheritsFrom(__classid(TImpDef))) return nullptr;
        if (D->InheritsFrom(__classid(TImpTypeDefRec))) {
            hUnit = static_cast<TImpTypeDefRec *>(D)->hImpUnit;
            N     = static_cast<TImpTypeDefRec *>(D)->ImpName;
        } else {
            hUnit = ((TImpDef *) D)->hUnit;
            N     = ((TImpDef *) D)->Name;
        }
        // imported value
        GetUnitImp(hUnit);

        return nullptr;
    }
    return static_cast<TTypeDef *>(D);
}
//------------------------------------------------------------------------------
TTypeValKind GetGlobalTypeValKind(int hDT) {
    TTypeDef *T = GetGlobalTypeDef(hDT);
    if (!T) return TTypeValKind::vkNone;
    return T->ValKind();
}
//------------------------------------------------------------------------------
TSegKind GetSegKindByName(PName Name) {
    int L = Name->GetStr().Length();

    if (L < 4 || L > 6) return TSegKind::None;

    // String S = "seg_";
    // for (int i = 2; i <= L; i++) {
    //     S += Name->Name[i];
    // }
    // PTypeInfo typeInfo = __typeinfo(TSegKind);
    // int idx = GetEnumValue(TSegKind, S);
    // if (idx < 0) idx = 0;
    // String S = Name->Name->SubString(2, L - 1);

   AnsiString key = Name->GetStr();

    std::map<AnsiString, TSegKind> SegMap = {
        { "text",  TSegKind::Text },
        { "itext", TSegKind::IText },
        { "idata", TSegKind::IData },
        { "bss",   TSegKind::BSS },
        { "tls",   TSegKind::TLS },
        { "pdata", TSegKind::PData },
        { "xdata", TSegKind::XData },
        { "tbss",  TSegKind::TBSS },
        { "rdata", TSegKind::RDATA }
    };

    auto it = SegMap.find(key);
    if (it != SegMap.end()) {
        return it->second;
    }

    return TSegKind::None;
}
//------------------------------------------------------------------------------
int __fastcall GetTypeSize(int hDef) {
    TTypeDef *T = GetGlobalTypeDef(hDef);
    if (!T) return -1;
    return T->Sz;
}
//------------------------------------------------------------------------------
void __fastcall SetProcAddInfo(int V) {
    if (V == -1) FhNextAddr = 0;
    if (V >= 1 && FVer >= verD7) FhNextAddr = V;
}
//------------------------------------------------------------------------------
void __fastcall FreeDCURecList(TDCURec *L) {
    while (L) {
        TDCURec *Tmp = L;
        L   = L->Next;
        delete Tmp;
    }
}
//------------------------------------------------------------------------------
TDCURec *__fastcall GetDCURecListEnd(TDCURec *L) {
    TDCURec *Result = L;
    while (Result) Result = Result->Next;
    return Result;
}
//------------------------------------------------------------------------------
void __fastcall SetExportNames(TDCURec *Decl) {
    FExportNames             = new TStringList;
    FExportNames->Sorted     = true;
    FExportNames->Duplicates = System::Types::TDuplicates::dupAccept; // For overloaded functions

    while (Decl) {
        // if (Decl->InheritsFrom(__classid(TNameFDecl)) && ((static_cast<TNameFDecl *>(Decl)->F & 0x40) != 0)) { // Decl->IsVisible(dlMain) it`s wrong, cause hides some names
        if (TNameFDecl *TND = dynamic_cast<TNameFDecl *>(Decl)) {
            if ((TND->F & 0x40) != 0) {
                // Name - we should export the original unmodified name
                FExportNames->AddObject(TND->GetExpName()->GetStr(), Decl);
                // was: FExportNames->AddObject(PName2String(Decl->GetName()), Decl);
            }
        }
        Decl = Decl->Next;
        // Decl = static_cast<TNameDecl *>(Decl->Next);
    }
}
//------------------------------------------------------------------------------
PName GetTypeName(int hDef) {
    TBaseDef *D = GetTypeDef(hDef);
    if (D) return D->FName;
    return nullptr;
}
//------------------------------------------------------------------------------
TTypeDef *__fastcall GetTypeDef(int hDef) {
    if (hDef <= 0 || hDef > FTypes->Count) return nullptr;
    return (TTypeDef *) FTypes->Items[hDef - 1];
}
//------------------------------------------------------------------------------

/**
 * TUnit.SetEnumConsts
 *
 * @param Decl
 */
void __fastcall SetEnumConsts(TDCURec** Decl) {
    TNameDecl **DeclP = reinterpret_cast<TNameDecl **>(Decl);
    TNameDecl **LastConstP = nullptr;
    TConstDecl *LastConst  = nullptr;
    TNameDecl *D = nullptr;

    int ConstCnt = 0;
    int V, CMin =0, CMax = 0;

    bool HasEq = false; // unused

    auto FlushConsts = [&]() {
        TTypeDef* TD = GetLocalTypeDef(LastConst->Value.hDT);
        TEnumDef* Enum = dynamic_cast<TEnumDef*>(TD);

        if (TD && Enum && Enum->NameTbl == nullptr &&
            TD->hDecl >= LastConst->hDecl + ConstCnt) {
            // Some paranoic tests:
            TScanState CP0;
            ChangeScanState(CP0, Enum->LH, 18);
            int Lo = ReadIndex();
            int Hi = ReadIndex();
            RestoreScanState(CP0);

            if ((Lo == CMin) && (Hi == CMax)) {
                *LastConstP = D;
                *DeclP = nullptr;
                DeclP = LastConstP;
                Enum->CStart = LastConst;

                if (ConstCnt + 100 > Hi - Lo) {
                    TList *NT = new TList;
                    NT->Count = Hi - Lo + 1; // Direct assignment to TList::Count/Capacity

                    while (LastConst) {
                        V = LastConst->Value.Val - Lo;
                        if (NT->Items[V] == nullptr) {
                            NT->Items[V] = LastConst;
                        }
                        LastConst = static_cast<TConstDecl*>(LastConst->Next);
                    }
                    Enum->NameTbl = NT;
                }
            }
        }

        LastConst = nullptr;
        ConstCnt = 0;
    };

    while (*DeclP) {
        /*if (!(*DeclP)->InheritsFrom(__classid(TNameDecl))) {
            DeclP = &(*DeclP)->Next;
            continue;
        }*/
        D = *DeclP;
        if (D->InheritsFrom(__classid(TConstDecl)) && static_cast<TConstDecl *>(D)->Value.ValSz == 0) {
            if (LastConst && LastConst->Value.hDT == static_cast<TConstDecl *>(D)->Value.hDT) {
                V = static_cast<TConstDecl *>(D)->Value.Val;
                ConstCnt++;
                if (V != CMax + 1) {
                    HasEq = true;
                }
                if (V > CMax) {
                    CMax = V;
                } else if (V < CMin) {
                    CMin = V;
                }
            } else {
                if (LastConst) FlushConsts();
                LastConstP = DeclP;
                LastConst  = static_cast<TConstDecl *>(D);
                ConstCnt   = 1;
                HasEq      = false;
                CMin       = LastConst->Value.Val;
                CMax       = LastConst->Value.Val;
            }
        } else {
            if (LastConst)
                FlushConsts();

            /*if (D->InheritsFrom(__classid(TTypeDecl)) && LastConst) {
                TTypeDef *TD = GetTypeDef(static_cast<TTypeDecl *>(D)->hDef);
                if (TD->InheritsFrom(__classid(TEnumDef))) {
                    TEnumDef *Enum = static_cast<TEnumDef *>(TD);
                    TList    *NT   = new TList;
                    NT->Capacity = ConstCnt;
                    *LastConstP  = D;
                    *DeclP       = nullptr;

                    while (LastConst) {
                        NT->Add(static_cast<void *>(LastConst));
                        LastConst = static_cast<TConstDecl *>(LastConst->Next);
                    }
                    Enum->NameTbl = NT;
                }
            }
            LastConst = nullptr;
            ConstCnt  = 0;*/
        }
        // DeclP = &(*D->Next);
        DeclP = reinterpret_cast<TNameDecl **>(&D->Next);
        // DeclP = &static_cast<TNameDecl *>(D->Next);
    }
}
//------------------------------------------------------------------------------
void __fastcall FillProcLocVarTbls() {
    if (!FLocVarTbl) return;

    PLocVarRec LVP    = FLocVarTbl;
    bool       isProc = false;
    TProcDecl *Proc   = NULL;
    int        iStart = 0;

    for (int i = 0; i < FLocVarCnt; i++) {
        bool wasProc = isProc;
        int  frame   = LVP->frame;
        TDCURec *D = NULL;
        isProc = false;
        if (!wasProc && LVP->sym) {
            D      = GetAddrDef(LVP->sym);
            isProc = (D->InheritsFrom(__classid(TProcDecl)));
            if (isProc) {
                if (Proc) {
                    Proc->FProcLocVarTbl = &FLocVarTbl[iStart];
                    Proc->FProcLocVarCnt = i - iStart;
                }
                Proc   = static_cast<TProcDecl *>(D);
                iStart = i;
            }
        }
        LVP++;
    }
    if (Proc) {
        Proc->FProcLocVarTbl = &FLocVarTbl[iStart];
        Proc->FProcLocVarCnt = FLocVarCnt - iStart;
    }
}
//------------------------------------------------------------------------------

/**
 * TUnit.ReadInDcpWin64Info
 * This kind of records was found in the .dcp files with Magic='PKX1' of Delphi 11
 * By now it will be ignored
 */
void __fastcall ReadInDcpWin64Info() {
    Byte  bFF = ReadByte();
    DWord F1 = ReadULong();
    DWord F2 = ReadULong();
}
//------------------------------------------------------------------------------

/**
 * TUnit.ReadSomeNameInfo28
 */
void __fastcall ReadSomeNameInfo28() {
    int V1;
    const int Kind = ReadUIndex();
    AnsiString sName, sUnit; // was String

    switch (Kind) {
        case 0: break;
        case 1:
            V1 = ReadUIndex();
            break;
        case 6:
            sName = ReadNDXStrX();
            sUnit = ReadNDXStrX();
            break;
        case 10:
            sName = ReadNDXStrX();
            V1    = ReadUIndex();
            break;
    }
}
//------------------------------------------------------------------------------

/**
 * TUnit.ConsumeEmbedded
 *
 * @return TDCURec*
 */
TDCURec *__fastcall ConsumeEmbedded() {
    TDCURec *Result = nullptr;

    if ((FMaxEmbedDepth > FEmbedLimit) || (FMaxEmbedDepth <= 0) || (FEmbedDepth >= FMaxEmbedDepth)) {
        return nullptr;
    }

    if (FEmbedDepth < FMaxEmbedDepth - 1) {
        printf("[Warning] ConsumeEmbedded: Skipped embedded lists %d..%d\n", FEmbedDepth + 1, FMaxEmbedDepth - 1); // DCUWarningFmt
    }

    FMaxEmbedDepth--;

    if (FEmbeddedLists) {
        Result = (*FEmbeddedLists)[FMaxEmbedDepth].List;
        (*FEmbeddedLists)[FMaxEmbedDepth].List = nullptr;
        (*FEmbeddedLists)[FMaxEmbedDepth].ListEnd = (*FEmbeddedLists)[FMaxEmbedDepth].List;
    } else {
        printf("Debug: ConsumeEmbedded: FEmbeddedLists is null\n");
    }

    return Result;
}
//------------------------------------------------------------------------------
// todo: needs InlineOp.h
/*TInlineDeclModifier * ReadInlineInfo(TDCURec * def) {
    printf("Debug: ReadInlineInfo: start\n");
    return nullptr;
}*/
//------------------------------------------------------------------------------
/*void __fastcall AddDefModifier(TDCURec *Def, TDeclModifier *M) {
    if (!M) return;
    if (Def) {
        if (Def->InheritsFrom(__classid(TNameDecl))) {
            TNameDecl *ND = static_cast<TNameDecl *>(Def);
            ND->AddModifier(M);
        }
        delete M;
    }
}*/
//------------------------------------------------------------------------------

/**
 * TUnit.ReadConstAddInfo
 * @param LastProcDecl
 * @return
 */
int __fastcall ReadConstAddInfo(TNameDecl *LastProcDecl) {
    // printf("Debug: ReadConstAddInfo: start\n");

    auto AddDefModifier = [](TDCURec* Def, TDeclModifier* M) {
        if (!M) return;
        /*if (Def && Def->InheritsFrom(__classid(TNameDecl)) ) {
            static_cast<TNameDecl *>(Def)->AddModifier(M);
        } else {
            delete M; // Just in case - shouldn't happen
        }*/
        if (TNameDecl* ND = dynamic_cast<TNameDecl*>(Def)) {
            ND->AddModifier(M);
        } else {
            delete M; // Just in case - shouldn't happen
        }
    };

    auto ReadAttributes = [&](TDCURec* Def) {
        // The compiler emits information about declaration attributes starting from XE6
        // (previous versions from D2010 just emitted links to attribute classes)
        int N = ReadUIndex();
        for (int i = 1; i <= N; ++i) {
            AddDefModifier(Def, new TAttributeDeclModifier);
        }
    };

    bool    brk = false;
    Byte    Tag, caiStop, b, *cPos;
    int     hDef, hDef1, hDef2, hDef3, hDef4, hDef5, hDT, F, IP;
    int     V1;
    int     V2;
    int     V3;
    int     V4;
    int     V5;
    int     cafInline; // was InlineMask
    int     cafBigVal;

    DWord   Len, Len1, V, hUnit;
    DWord	W1, W2;
    int     hDef11, hDef12, hDef13, hDef14, hDef15;
    int     IP2, Z;
    AnsiString  S; // was String
    int Kind;
    int Index;

    TDCURec *Def; // -sg: needed?

    int Result = -1;
    if (FVer <= verD7 || FVer >= verK1) {
        caiStop = 0x06;
        // ReadByte();
        // ReadUIndex();
        // ReadByte();
        // ReadByte();
        return Result;
    }

    caiStop = 0x0D;
    if (FVer >= verD2005) {
        caiStop = 0x0F;
        if (FVer >= verD2009) {
            caiStop = 0xFF;
        }
    }

    // -sg: test:
    /*if (FVer <= verD7 || FVer >= verK1) {
        caiStop = 0x06;
        // ReadByte();
        // ReadUIndex();
        // ReadByte();
        // ReadByte();
        return Result;
    }
    if (FVer >= verD11) {
        caiStop = 0x18;
    } else if (FVer >= verD10_4) {
        caiStop = 0x17;
    } else if (FVer >= verDXE1) {
        caiStop = 0x16;
    } else if (FVer >= verD2009) {
        caiStop = 0x15;
    } else if (FVer >= verD2005) {
        caiStop = 0x0F;
    } else {
        caiStop = 0xD;
    }*/

    /*caiStop = 0xD;
    if (FVer >= verD2005) {
        caiStop = 0xF;
        if (FVer >= verD2009) caiStop = 0xFF;
    }*/

    while (true) {
        Tag = ReadByte();
        // printf("Debug: ReadConstAddInfo: Tag: %lX\n", Tag);
        // check it before case to skip the tags for the higher versions
        if (Tag >= caiStop) break;
        switch (Tag) {
            case 0x01:
                Result = ReadUIndex();
                // RefAddrDef(Result);
                F = ReadUIndex();
                Def = GetAddrDef(Result);
                if (Def && Def->InheritsFrom(__classid(TNameDecl))) {
                    TNameDecl *ND = static_cast<TNameDecl *>(Def);
                    ND->ConstAddInfoFlags = F;
                }

                if ((FVer >= verD2006) && (FVer <= verD10) && (FVer < verK1) &&
                    !((FPlatform == dcuplIOSEmulator || FPlatform == dcuplIOSDevice || FPlatform == dcuplIOSDevice64 ||
                       FPlatform == dcuplAndroid))) {
                    if ((F & 0x1000000) != 0)
                        IP = ReadUIndex();
                }

                if (IsMSIL) {
                    if (FVer >= verD2005) {
                       if ((F & 0x10000) !=0) { // or 0x20000, because F was = 0x30000
                         Len = ReadUIndex();
                         SkipBlock(Len); // A C# code associated with Result
                       }
                    }
                    Len = ReadUIndex();
                    for (int i = 1; i <= Len; i++) {
                        hDef = ReadUIndex();
                        RefAddrDef(hDef);
                        V = ReadUIndex();
                        S = ReadNDXStr();
                        if (S != "") {
                            Len1 = ReadUIndex();
                            for (int j = 1; j <= Len1; j++) {
                                hDef1 = ReadUIndex();
                                RefAddrDef(hDef1); // It seems that it's required to reserve addr index
                            }
                        }
                    }
                }

                if (FVer >= verD2005 && FVer < verK1) {
                    cafInline = 0x80000;
                    cafBigVal = 0x100000;

                    if (FVer >= verD2009) {
                        if (F & 0x800000)
                            IP2 = ReadUIndex(); // hUsedCl - Attribute class used for the declaration
                        if (F & 0x1) { // Deprecated
                            // S = ReadNDXStr();
                            AddDefModifier(Def, new TDeprecatedDeclModifier(ReadNDXStrRef()));
                        }
                        if (FVer >= verDXE6 && (F & 0x80000000) != 0)
                            ReadAttributes(Def);

                        cafInline = 0x40000;
                        cafBigVal = 0x80000;
                    }
                    /*else if (FVer >= verD2006) {
                        if (F & 0x1000000)
                            IP = ReadUIndex();
                    }*/

                    if ((F & cafInline) != 0) {
                        // Very complex structure - corresponds to the new (Ver>=8) inline directive.
                        // Fortunately, we can completely ignore all this info, because it is duplicated
                        // as a regular procedure info even for inlines

                        // AddDefModifier(Def, ReadInlineInfo(Def));
                        // todo: replace below with ReadInLineInfo() above
                        if (FVer >= verD2006 && FVer < verK1) {
                            ReadUIndex();
                            ReadUIndex();
                        }
                        Len = ReadUIndex();
                        SkipBlock(Len + sizeof(Byte));
                        for (int i = 1; i <= 5; i++) ReadUIndex();

                        if (FVer >= verDXE2 && FVer < verK1)
                            V = ReadUIndex();

                        V = ReadUIndex();

                        // RefAddrDef(V);

                        Len = ReadUIndex();
                        if (FVer >= verD2009) {
                            ReadUIndex();
                            ReadUIndex();
                            Len1 = ReadUIndex();
                            SkipBlock(Len1 * sizeof(int));
                        }

                        for (int i = 1; i <= Len; i++) {
                            if (FVer >= verD2009) {
                                V = ReadUIndex(); // Flags
                                if (V == 0)
                                    V = ReadUIndex(); // hAddr
                            } else
                                V = ReadUIndex(); // hAddr

                            RefAddrDef(V); // It seems that it's required to reserve addr index

                            if (FVer >= verD2009) {
                                ReadUIndex();
                                ReadUIndex();
                                V = ReadUIndex();
                            }

                            Z = ReadUIndex();
                            if (FVer >= verD2010)
                                ReadUIndex();

                            // if (FVer >= verD2009 && Z) ReadUIndex();

                            if (FVer >= verD2009) {
                                for (int j = 1; j <= Z; j++)
                                    ReadUIndex();
                            }
                        }

                        Len = ReadUIndex();
                        for (int i = 1; i <= Len; i++) {
                            V = ReadUIndex();
                            if (FVer >= verD2009) {
                                // Ok = true;
                                switch (V) {
                                    case 1:
                                        V = ReadUIndex();
                                        if (FVer >= verDXE1) // Perhaps it's required for lower versions too
                                            RefAddrDef(V);
                                        V = 1 + static_cast<int>(FVer >= verDXE1);
                                        // if (FVer >= verDXE1) V = 2;
                                        // else V = 1;
                                        break;
                                    case 2: V = 1; break;
                                    case 3: V = 3; break;
                                    case 4: V = 2; break;
                                    case 5: V = 4; break;
                                    case 6: V = 1; break;
                                    case 7:
                                        if (FVer >= verDXE8)
                                            V = 1 + static_cast<int>(FVer >= verD10_1);
                                        break;
                                    default: printf("[Error] ReadConstAddInfo: Unexpected TConstAddInfo.1 LF value: %d\n", V); break;
                                }
                                for (int j = 1; j <= V; j++) ReadUIndex();
                            } else {
                                if (V == 1) {
                                    ReadUIndex();
                                    ReadUIndex();
                                }
                                V = ReadUIndex();
                            }
                        }
                        Len = ReadUIndex(); // Number of units defs from which are used in this def
                        for (int i = 1; i <= Len; i++) {
                            hUnit = ReadUIndex();
                            // RefAddrDef(hUnit);
                            Len1 = ReadUIndex();
                            for (int j = 1; j <= Len1; j++) {
                                V = ReadUIndex();
                                if (hUnit) continue; // Import from another unit - don't care
                                RefAddrDef(V);
                            }
                        }

                        if (FVer >= verD2006) {
                            Len = ReadUIndex();
                            for (int i = 1; i <= Len; i++) ReadUIndex();
                            if (FVer >= verD2009) {
                                ReadUIndex();
                                V = ReadUIndex();
                                RefAddrDef(V); // AppMethod: System.Threading
                                ReadUIndex();
                            }
                        }
                    }

                    if (FVer >= verD2005 && (F & cafBigVal) != 0) {
                        IP = ReadUIndex();
                    }
                    /*if (FVer >= verD2009) {
                        if (F & 0x80000) ReadUIndex();
                    } else {
                        if (F & 0x100000) ReadUIndex();
                    }*/
                }
                break;
            case 0x4:
                if (FVer >= verD2006 && FVer < verK1) {
                    V = ReadUIndex();
                    V = ReadUIndex();
                }
                break;
            case 0x6:
                Result = ReadUIndex();
                hDT    = ReadUIndex();
                V      = ReadUIndex();
                hDef1  = ReadUIndex();
                break;
            case 0x7:
                Result = ReadUIndex();
                hDef1  = ReadUIndex();
                hDef2  = ReadUIndex();
                V      = ReadUIndex();
                break;
            case 0x8:
                if (FVer >= verD11 && FVer < verK1) {
                    Result = ReadUIndex();
                    Def    = GetAddrDef(Result);
                    AddDefModifier(Def, new TXMLDocDeclModifier(ReadNDXStrRef()));
                }
                break;
            case 0x9:
                Result = ReadUIndex();
                hDT    = ReadUIndex();
                break;
            case 0xA:
                Result = ReadUIndex();
                Def    = GetAddrDef(Result);
                V      = ReadUIndex();
                F      = ReadUIndex();
                hDT    = 0;
                if (F & 0x1)
                    hDT = ReadUIndex();
                hDef1 = 0;
                if (F & 0x2)
                    hDef1 = ReadUIndex();
                V2 = 0;
                if (F & 0x4)
                    V2 = ReadUIndex();
                V3 = 0;
                if (F & 0x8)
                    V3 = ReadUIndex();
                V4 = 0;
                if (F & 0x10)
                    V4 = ReadUIndex();
                hDef5 = 0;
                if (F & 0x20)
                    hDef5 = ReadUIndex();
                if (F & 0x40) {
                    AddDefModifier(Def, new TExtraArgsDeclModifier); // TExtraArgsDeclModifier.Read
                    /*Len = ReadUIndex();
                    for (int i = 1; i <= Len; i++) {
                        S   = ReadNDXStr();
                        V   = ReadUIndex();
                        V1  = ReadUIndex();
                        hDT = ReadUIndex();
                    }*/
                }
                if (F & 0x80) {
                    V  = ReadUIndex();
                    V1 = ReadUIndex();
                    V2 = ReadUIndex();
                }
                if (F & 0x100) {
                    // S = ReadNDXStr();
                    AddDefModifier(Def,new TGeneratedNameDeclModifier(ReadNDXStrRef()));
                }
                if (F & 0x200)
                    S = ReadNDXStr();
                hDef11 = 0;
                if (F & 0x400) {
                    hDef11 = ReadUIndex();
                    if (IsMSIL) RefAddrDef(hDef11);
                }
                hDef12 = 0;
                if (F & 0x800) {
                    hDef12 = ReadUIndex();
                    if (IsMSIL) RefAddrDef(hDef12);
                }
                hDef13 = 0;
                if (F & 0x1000) {
                    hDef13 = ReadUIndex();
                    if (IsMSIL) RefAddrDef(hDef13);
                }
                hDef14 = 0;
                if (F & 0x2000) {
                    hDef14 = ReadUIndex();
                    if (IsMSIL) RefAddrDef(hDef14);
                }
                hDef15 = 0;
                if (F & 0x4000) {
                    hDef15 = ReadUIndex(); // MSIL 9 only?
                    if (IsMSIL) RefAddrDef(hDef15);
                }
                break;
            case 0xC:
                Result = ReadUIndex();
                V1     = ReadUIndex();
                V2     = ReadUIndex();
                break;
            case 0xD:
                if (FVer < verD2005 || FVer >= verK1) return Result;
                // imported unit module information (FileName and version)?
                Result = ReadUIndex();
                S      = ReadNDXStr();
                if (IsMSIL)
                    SetUnitPackageInfo(Result,S);
                break;
            case 0x10:
                if (FVer < verD2009 || FVer >= verK1) return Result;
                V1 = ReadUIndex();
                V2 = ReadUIndex();
                V3 = ReadUIndex();
                break;
            case 0x11:
                // The record links explicitly links drStrConstRec to drVarC, which uses its memory
                if (FVer < verDXE4 || FVer >= verK1) return Result;
                V1 = ReadUIndex();
                RefAddrDef(V1); // Seems that it's required to reserve addr index
                V2 = ReadUIndex();
                RefAddrDef(V2); // Seems that it's required to reserve addr index
                break;
            case 0x12:
                if (FVer < verD2009 || FVer >= verK1) return Result;
                V1 = ReadUIndex();
                V2 = ReadUIndex();
                break;
            case 0x13:
                if (FVer < verD2009 || FVer >= verK1) return Result;
                V1 = ReadUIndex();
                RefAddrDef(V1); // It seems that it's required to reserve addr index
                V2 = ReadUIndex();
                V3 = ReadUIndex();
                S  = ReadNDXStr(); // $EXTERNALSYM
                S  = ReadNDXStr(); // ??
                S  = ReadNDXStr(); // $OBJTYPENAME
                break;
            case 0x14:
                if (FVer < verD2009 || FVer >= verK1) return Result;
                V1 = ReadUIndex();  // Addr index of the aux field
                // RefAddrDef(V1); // ???
                V2  = ReadUIndex();
                Len = ReadUIndex();
                for (int i = 1; i <= Len; i++) {
                    // V1 = ReadUIndex();
                    // V2 = ReadUIndex();
                    // V3 = ReadUIndex();
                    // S  = ReadNDXStr();
                    Kind = ReadUIndex(); // 0-record,1-array
                    hDT = ReadUIndex();
                    Index = ReadUIndex();
                    if (Kind == 0)
                        S = ReadNDXStr();
                }
                break;
            case 0x15:
                if (FVer < verDXE2 || FVer >= verK1) return Result;
                V = ReadUIndex();
                V1 = ReadUIndex();
                // RefAddrDef(V1);
                V2 = ReadUIndex();
                // V3 = ReadUIndex();
                break;
            case 0x16:
                if (!(FVer >= verD11 && FVer < verK1)) return Result;
                Result = ReadUIndex();
                S = ReadNDXStr();
                break;
            case 0x17:
                if (!(FVer >= verD12 && FVer < verK1)) return Result;
                Result = ReadUIndex();
                for (int i = 0; i <= 1; i++)
                    ReadSomeNameInfo28(); // In fact, one of the kinds is always 0
                break;
            default:
                break;
        }
    }

    if (Tag != caiStop) {
        printf("Debug: ReadConstAddInfo: Unexpected Tag=$%x in TConstAddInfoRec\n", Tag); // DCUErrorFmt
        return Result;
    }

    return Result;
}
//------------------------------------------------------------------------------
// TUnit.SetUnitPackageInfo
void __fastcall SetUnitPackageInfo(int hDecl, const String sInfo) {
    TDCURec *D = GetAddrDef(hDecl);
    if (TUnitImpDef* UnitImp = dynamic_cast<TUnitImpDef*>(D)) {
        UnitImp->sPackage = sInfo;
    }
}
//------------------------------------------------------------------------------
// int CurDeclNo = 0;

/**
 * The record contains information about dependencies of imported subroutines
 * By now it will be ignored
 * In fact, Embarcadero doesn't explain its purpose
 */
void __fastcall ReadDependencyInfo() {
    TNDX Cnt = ReadUIndex();
    for (int i = 0; i < Cnt; i++) {
        Byte Z = ReadByte();
        if (Z) printf("Unexpected DependencyInfoItem #%d Z=%d", i, Z);
        PShortName sName = ReadShortName();
        // PName sName = ReadName();
        Word  W     = ReadWord();
        TNDX  NDX   = ReadUIndex();
        TNDX  L     = ReadUIndex();

        for (int j = 0; j < L; j++) {
            NDX = ReadUIndex();
        }
    }
}
//------------------------------------------------------------------------------

/**
 * TUnit.IncEmbedDepth
 * @param HeadBuf
 * @return
 */
PDCURec IncEmbedDepth(TDCURec *&HeadBuf) {
    // todo: review
    FEmbedDepth++;

    try {
        if (FEmbedDepth > FMaxEmbedDepth) {
            FMaxEmbedDepth = FEmbedDepth;
            int Lim = FMaxEmbedDepth * 2;
            if (Lim < 16) Lim = 16;
            FEmbeddedLists = static_cast<PEmbeddedListInfTbl>(ReallocMemory(FEmbeddedLists, Lim * sizeof(TEmbeddedListInf)));
            for (int i = 0; i < FEmbedLimit; i++) {
                if (!(*FEmbeddedLists)[i].List)
                    (*FEmbeddedLists)[i].ListEnd = (*FEmbeddedLists)[i].List;
            }
            for (int i = FEmbedLimit; i < Lim; i++) {
                (*FEmbeddedLists)[i].List    = nullptr;
                (*FEmbeddedLists)[i].ListEnd = (*FEmbeddedLists)[i].List;
            }
            FEmbedLimit = Lim;
        }
    } catch (Exception &e) {
        // todo: test
        String trace = FormatExceptionWithStackTrace(&e);
        printf("Exception: %s\n", AnsiString(trace).c_str());
    }/* catch (const std::exception &e) {
        printf("Exception: %s\n", e.what());
    } catch (...) {
        printf("Unknown exception in IncEmbedDepth\n");
    }*/

    HeadBuf = FEmbeddedLists[FEmbedDepth - 1]->List;
    if (!(*FEmbeddedLists)[FEmbedDepth - 1].List) {
        return HeadBuf;
    }
    return (*FEmbeddedLists)[FEmbedDepth - 1].ListEnd;
}

/*PDCURec IncEmbedDepth(TDCURec *&HeadBuf) {
    FEmbedDepth++;

    try {
        if (FEmbedDepth > FMaxEmbedDepth) {
            FMaxEmbedDepth = FEmbedDepth;
            int Lim = FMaxEmbedDepth * 2;
            if (Lim < 16) Lim = 16;
            FEmbeddedLists = static_cast<PEmbeddedListInfTbl>(ReallocMemory(FEmbeddedLists, Lim * sizeof(TEmbeddedListInf)));
            for (int i = 0; i < FEmbedLimit; i++) {
                if (!(FEmbeddedLists)[i]->List)
                    FEmbeddedLists[i]->ListEnd = FEmbeddedLists[i]->List;
            }
            for (int i = FEmbedLimit; i < Lim; i++) {
                FEmbeddedLists[i]->List    = nullptr;
                FEmbeddedLists[i]->ListEnd = FEmbeddedLists[i]->List;
            }
            FEmbedLimit = Lim;
        }
    } catch (Exception &e) {
        printf("Exception: %s\n", AnsiString(e.Message).c_str());
    } catch (const std::exception &e) {
        printf("Exception: %s\n", e.what());
    } catch (...) {
        printf("Unknown exception in IncEmbedDepth\n");
    }

    HeadBuf = FEmbeddedLists[FEmbedDepth - 1]->List;
    if (!FEmbeddedLists[FEmbedDepth - 1]->List) {
        return HeadBuf;
    }
    return FEmbeddedLists[FEmbedDepth - 1]->ListEnd;
}*/
//------------------------------------------------------------------------------

/**
 *
 * @param LK TDeclListKind
 * @param Owner
 * @param Result
 */
void __fastcall ReadDeclList(Byte LK, TDCURec *Owner, TDCURec **Result) {
    int  V;
    int  EmbEndCnt  = 0;

    bool brk = false;

    // bool WasEmbEnd = false; // For MSIL only
    // TDCURec *Embedded = nullptr;
    // TDCURec **DeclEnd       = Result;

    *Result = nullptr;

    TNameDecl *LastProcDecl = nullptr;
    FhNextAddr              = 0;

    PDCURec DeclEnd = *Result;
    PDCURec EmbLEnd;
    TDCURec *EmbedBuf;

    while (true) {
        Byte Tag1 = FixTag(Tag);
        TDCURec *Decl = nullptr;
        TDCURec *Rec = nullptr;
        switch (Tag1) {
            case drType:
                // TTypeDecl.Create(LK in [dlArgs,dlArgsT,dlFields,dlClass,dlInterface,dlDispInterface]);
                Decl = new TTypeDecl;
                break;
            case drTypeP:
                Decl = new TTypePDecl;
                break;
            case drConst:
                Decl = new TConstDecl;
                break;
            case drResStr:
                Decl = new TResStrDef;
                break;
            case drSysProc:
                if (FVer >= verD8 && FVer < verK1)
                    Decl = new TSysProc8Decl;
                else
                    Decl = new TSysProcDecl;
             break;
            case drProc:
                LastProcDecl = new TProcDecl(ConsumeEmbedded(), false);
                Decl = LastProcDecl;
                // Decl = new TProcDecl(ConsumeEmbedded(), false);
                // LastProcDecl = Decl;
                break;
            case drEmbeddedProcStart:
                if ((IsMSIL || (FVer >= verD2009 && FVer < verK1)) && EmbEndCnt > 0) {
                    // Escape up from parameter list processing for default values' consts
                    EmbEndCnt--; // Just ignore and continue
                } else {
                    EmbLEnd = IncEmbedDepth(EmbedBuf);
                    Tag = ReadTag();
                    ReadDeclList(dlEmbedded, nullptr, &EmbLEnd);
                    FEmbedDepth--;

                    if (Tag != drEmbeddedProcEnd) printf("[Error] ReadDeclList: drEmbeddedProcStart: Embedded Stop Tag\n"); // TagError

                    // try to fix the local types relocation problem of XE
                    if (FVer >= verDXE1 && FVer < verK1) {
                        RegisterEmbeddedTypes(EmbLEnd, FEmbedDepth + 1);
                    }

                    if (EmbedBuf) {
                        if (EmbLEnd == EmbedBuf) {
                            FEmbeddedLists[FEmbedDepth]->List = EmbedBuf;
                        }

                        EmbLEnd = GetDCURecListEnd((TDCURec*)EmbLEnd);
                        FEmbeddedLists[FEmbedDepth]->ListEnd = EmbLEnd;
                    }
                }

                /*if ((IsMSIL || (FVer >= verD2009 && FVer < verK1)) && WasEmbEnd)
                    WasEmbEnd = false; // Just ignore and continue
                else {
                    FEmbedDepth++;
                    if (Embedded) {
                        if (!IsMSIL) printf("Warning: Duplicate embedded list\n");
                        Tag           = ReadTag();
                        TDCURec *le = GetDCURecListEnd(Embedded);
                        ReadDeclList(dlEmbedded, &le);
                    } else {
                        Tag = ReadTag();
                        ReadDeclList(dlEmbedded, &Embedded);
                    }
                    FEmbedDepth--;
                    if (Tag != drEmbeddedProcEnd) printf("Warning: Embedded Stop Tag\n");
                    // try to fix the local types relocation problem of XE
                    if (FVer >= verDXE1 && FVer < verK1) RegisterEmbeddedTypes(Embedded, FEmbedDepth + 1);
                }*/
                break;
            case drEmbeddedProcEnd:
                // verD5 was observed, but may be in prev ver. too
                if (!((LK == dlArgsT && FVer > verD3) || (LK == dlArgs && FVer > verD3))) {
                    // Temp. - this tag can mark the const definition used as an interface arg. default value and also as proc. arg. default value
                    brk = true;
                    break;
                }
                if (IsMSIL || (FVer >= verD2009 && FVer < verK1)) {
                    // For MSIL only. And Ver>=verD2009 too
                    // drEmbeddedProcEnd - drEmbeddedProcStart mark block of const defs
                    // In fact, they mark escape up from the data type declaration place
                    // (when escaping from a data type defined inside a top level or embedded procedure,
                    // the depth of escapes increases).
                    // We'll try to just ignore them
                    // Beginning from D2009 they started to close the escapes by drEmbeddedProcEnd before the stop tag
                    EmbEndCnt++;
                }
                break;
            case drVar:
                if (LK == dlArgs || LK == dlArgsT)
                    Decl = new TLocalDecl(LK);
                else if (LK == dlMain && FVer >= verDXE2 && FPlatform == dcuplWin64)
                    Decl = new TVarVDecl;
                else
                    Decl = new TVarDecl;
              break;
            case drThreadVar:
                Decl = new TThreadVarDecl;
                break;
            case drExport:
                Decl = new TExportDecl;
                break;
            case drVarC:
                Decl = new TVarCDecl(false);
                break;
            case arVar:
            case arResult:
            case arFld:
                Decl = new TLocalDecl(LK);
                break;
            case arVal:
                Decl = new TLocalValDecl(LK);
                break;
            case arAbsLocVar:
                if (LK == dlMain || LK == dlMainImpl)
                    Decl = new TAbsVarDecl;
                else
                    Decl = new TLocalDecl(LK);
                break;
            case arLabel:
                Decl = new TLabelDecl;
                break;
            case arMethod:
            case arConstr:
            case arDestr:
                Decl = new TMethodDecl(LK);
                break;
            case arClassVar:
                if (!(FVer >= verD2006 && FVer < verK1))
                    brk = true;
                else
                    Decl = new TClassVarDecl(LK);
                break;
            case arProperty:
                if (LK == dlDispInterface)
                    Decl = new TDispPropDecl(LK);
                else
                    Decl = new TPropDecl;
                break;
            case arCDecl:
            case arPascal:
            case arStdCall:
            case arSafeCall:
                // Skip it
                break;
            case arSetDeft:
                Decl = new TSetDeftInfo;
                break;
            case drStop2:
                if (FVer >= verD8 && FVer < verK1)
                    ReadULong();
                break;
            case drStrConstRec:
                if (!(FVer >= verD8 && FVer < verK1))
                    brk = true;
                else
                    Decl = new TStrConstDecl;
                break;
            case drSpecVar:
                if ((FVer >= verD2006 && FVer < verK1))
                    brk = true;
                else
                    Decl = new TSpecVar;
                break;
            // Type definitions
            case drRangeDef:
            case drChRangeDef:
            case drBoolRangeDef:
            case drWCharRangeDef:
            case drWideRangeDef:
                Rec = new TRangeDef;
                break;
            case drEnumDef:
                Rec = new TEnumDef;
                break;
            case drFloatDef:
                Rec = new TFloatDef;
                break;
            case drPtrDef:
                Rec = new TPtrDef;
                break;
            case drTextDef:
                Rec = new TTextDef;
                break;
            case drFileDef:
                Rec = new TFileDef;
                break;
            case drSetDef:
                Rec = new TSetDef;
                break;
            case drShortStrDef:
                Rec = new TShortStrDef;
                break;
            case drStringDef:
            case drWideStrDef:
                Rec = new TStringDef;
                break;
            case drArrayDef:
                Rec = new TArrayDef(false);
                break;
            case drVariantDef:
                Rec = new TVariantDef;
                break;
            case drObjVMTDef:
                Rec = new TObjVMTDef;
                break;
            case drRecDef:
                Rec = new TRecDef;
                break;
            case drProcTypeDef:
                Rec = new TProcTypeDef;
                break;
            case drObjDef:
                Rec = new TObjDef;
                break;
            case drClassDef:
                Rec = new TClassDef;
                break;
            case drMetaClassDef:
                if (!(FVer >= verD8 && FVer < verK1))
                    brk = true;
                else
                    Rec = new TMetaClassDef;
                break;
            case drInterfaceDef:
                new TInterfaceDef;
                break;
            case drVoid:
                // May be end of interface
                new TVoidDef;
                break;
            // End of Type definitions
            case drCBlock:
                if (LK != dlMain)
                    brk = true;
                else {
                    if (FDataBlPtr) printf("Warning: 2nd Data block\n");
                    FDataBlSize = ReadUIndex();
                    FDataBlPtr  = ReadMem(FDataBlSize);
                }
                break;
            case drFixUp:
                LoadFixups();
                break;
            // The following tables are present only when debug info is on
            case drCodeLines:
                LoadCodeLines();
                break;
            case drLinNum:
                LoadLineRanges();
                break;
            case drStrucScope:
                LoadStrucScope();
                break;
            case drLocVarTbl:
                LoadLocVarTbl();
                break;
            case drSymbolRef: // Present if symbol info is on
                LoadSymbolInfo();
                break;
            case drUnitAddInfo: // ver70
                if (!(FVer >= verD7 && FVer < verK1))
                    brk = true;
                else
                    Decl = new TUnitAddInfo; // ReadUnitAddInfo;
                break;
            case drConstAddInfo:
                if (!((FVer >= verD7 && FVer < verK1) || (FVer >= verK3)))
                    brk = true;
                else
                    ReadConstAddInfo(LastProcDecl); // used for deprecated and other additional information
                break;
            case drProcAddInfo:
                if (!(FVer >= verD7 && FVer < verK1))
                    brk = true;
                else {
                    V = ReadIndex();
                    SetProcAddInfo(V);
                }
                break;
            case drNextOverload:
                if (!((FVer >= verDXE7) && (FVer < verK1))) break;
                V = ReadUIndex();  //W as observed after overloaded proc header before args
                // contains index of the next overload of the procedure, 0 => the last overload
                RefAddrDef(V);
                break;
            case drDependencyInfo:
                if (!((FVer >= verD10) && (FVer < verK1))) break;
                ReadDependencyInfo();
                break;
            case drORec:
                if (!(FVer >= verD8 && FVer < verK1))
                    brk = true;
                else {
                    if (FVer >= verD2009)
                        Decl = new TORecDecl;
                    else
                        ReadUIndex();
                }
                break;
            case drCPPFlags:
                if (!(FVer >= verD3 && FVer < verK1))
                    brk = true;
                else {
                    ReadByte(); // Flags: EXTERNALSYM ^ 4, NODEFINE ^ 8, NOINCLUDE ^ 0x10, OBJTYPENAME ^ 0x20
                    ReadUIndex(); // Addr
                }
                break;
            /*case drInfo98:
                if (!(FVer >= verD8 && FVer < verK1))
                    brk = true;
                else {
                    ReadUIndex();
                    ReadUIndex();
                }
                break;*/
            case drCLine: // Lines of C text, just ignore them by now
                if (!(FVer >= verD2006 && FVer < verK1))
                    brk = true;
                else {
                    if (FVer >= verDXE1 && FVer < verK1)
                        int X = ReadByte();
                    V = ReadUIndex(); // Length of the line
                    SkipBlock(V);     // Line chars
                }
                break;
            case drA1Info: // Some record of 6 indices, ignore it completely
                if (!(FVer >= verD2006 && FVer < verK1))
                    brk = true;
                else {
                    ReadUIndex();
                    ReadUIndex();
                    ReadUIndex();
                    ReadUIndex();
                    V = ReadUIndex();
                    for (int i = 1; i <= V; i++) ReadUIndex();
                }
                break;
            case drA2Info:
                if (!(FVer >= verD2006 && FVer < verK1))
                    brk = true;
                // No data for this tag
                break;
            case arCopyDecl:
                if (FVer >= verD2006 && FVer < verK1) {
                    Decl = new TCopyDecl;
                }
                break;
            case drA5Info:
                if (!(FVer >= verD2009 && FVer < verK1))
                    brk = true;
                // No data for this tag
                break;
            case drA6Info:
                if (!(FVer >= verD2009 && FVer < verK1))
                    brk = true;
                else
                    Decl = new TA6Def;
                break;
            case drA7Info:
                if (!(FVer >= verD2009 && FVer < verK1))
                    brk = true;
                else // todo
                    new TTemplateParmsDeclModifier; // was: new TA7Def;
                break;
            case drA8Info:
                if (!(FVer >= verD2009 && FVer < verK1))
                    brk = true;
                else
                    ReadUIndex(); // !!!M.b. some DCU record to be created
                break;
            case drA9Info:
                if (!(FVer >= verDXE4 && FVer < verK1))
                    brk = true;
                else
                    ReadUIndex(); // !!!M.b. some DCU record to be created
                break;
            case drDynArrayDef:
                if (!(FVer >= verD2009 && FVer < verK1))
                    brk = true;
                else
                    Rec = new TDynArrayDef;
                break;
            case drTemplateArgDef:
                if (!(FVer >= verD2009 && FVer < verK1))
                    brk = true;
                else
                    Rec = new TTemplateArgDef;
                break;
            case drTemplateCall:
                if (FVer >= verD2009 && FVer < verK1) {
                    Rec = new TTemplateCall;
                }
                break;
            case drUnicodeStringDef:
                if (!(FVer >= verD2009 && FVer < verK1))
                    brk = true;
                else
                    Rec = new TStringDef;
                break;
            case arAnonymousBlock:
                if (!(FVer >= verD2010 && FVer < verK1))
                    brk = true;
                else {
                    ReadUIndex(); // !!!M.b. some DCU record to be created
                    ReadUIndex();
                }
                break;
            case drDelayedImpInfo:
                if (!(FVer >= verD2009 && FVer < verK1))
                    brk = true;
                else
                    Decl = new TDelayedImpRec;
                break;
            case drSegInfo: // XE2
                if (!(FVer >= verDXE2 && FVer < verK1))
                    brk = true;
                // todo: review:
                if (FSegKindTbl) printf("[Error] 2nd Segment table"); // DCUError
                V = ReadUIndex();
                FSegCnt = V;
                FSegKindTbl = static_cast<PSegKindTbl>(AllocMem(V * sizeof(TSegKind)));
                for (int i = 0; i < V; i++) {
                  *FSegKindTbl[i] = GetSegKindByName(ReadName());
                  ReadByte();
                  ReadUIndex();
                }
                break;
            case drAddrToSegInfo:
                if (!(FVer >= verDXE2 && FVer < verK1))
                    brk = true;
                else
                    LoadAddrToSegInfo();
                break;
            case drAssemblyData:
                if (!(FromPackage && IsMSIL))
                    brk = true;
                else
                    Decl = new TAssemblyData;
                break;
            case arFinalFlag:
                if (!(FVer >= verDXE3 && FVer < verK1))
                    brk = true;
                else
                    V = ReadUIndex();
                break;
            default:
                brk = true;
                break;
        }
        if (brk) break;
        // __finally
        if (Decl) {
            DeclEnd = Decl;
            DeclEnd = Decl->Next;
        } else if (Rec) {
            Rec->ListAppend(FOtherRecords);
        }
        Tag = ReadTag();
    }
    /*if (Embedded) {
        if ((IsMSIL && LK == dlEmbedded) || (FVer >= verD2010 && FVer <= verK1 && (LK == dlArgs || LK == dlArgsT)) ||
            (LK == dlEmbedded && !*Result)) {
            // A lot of files contain additional drEmbeddedProcStart - drEmbeddedProcEnd
            // brackets for aux record
            *DeclEnd = Embedded;
        } else {
            FreeDCURecList(Embedded);
            printf("Error: Unused embedded list\n");
        }
    }*/
}
//------------------------------------------------------------------------------

enum class TDeclSepFlags : int {
    Comma         = 0,
    Last          = 1,
    NoFirst       = 2,
    NL            = 3,
    SoftNL        = 4,
    SmallSameNL   = 5,
    OfsProc       = 6
};

// TDeclSepFlags
#define dsComma         0
#define dsLast          1
#define dsNoFirst       2
#define dsNL            3
#define dsSoftNL        4
#define dsSmallSameNL   5
#define dsOfsProc       6

// aka DeclSecNames / TDeclSecKind
String SecNames[] = {
        "",               // 0
        "label",          // 1
        "const",          // 2
        "type",           // 3
        "var",            // 4
        "threadvar",      // 5
        "resourcestring", // 6
        "exports",        // 7
        "",               // 8
        "private",        // 9
        "protected",      // 10
        "public",         // 11
        "published"       // 12
};

void __fastcall ShowDeclList(Byte LK, TDCURec* Decl, String& OutS) {
    String SecN, S;

    OutS = "";
    int DeclCnt = 0;

    while (Decl) {
        bool Visible = Decl->IsVisible(LK);
        if (Visible) {
            Byte SK = Decl->GetSecKind();
            if (DeclCnt > 0) {
                OutS += ";";
                OutLog1(";\n"); // !!! prototype separator
            }

            if (SK >= 9 && SK <= 12)
                ActiveScope = SK;
            else
                ActiveScope = 0;

            // const
            if (SK == 2) {
                ActiveInfo = 2;
            } else if (SK == 3) {
                // type
                ActiveInfo = 3;
            } else if (SK == 4) {
                // var
                ActiveInfo = 4;
            } else if (SK == 5) {
                // threadvar
                ThreadVar  = true;
                ActiveInfo = 5;
            } else if (SK == 6) {
                // resourcestring
                ActiveInfo = 6;
            }
            
            SecN = SecNames[SK]; // SecN = DeclSecNames[LK]
            if (SecN != "") OutLog2("%s ", SecN.c_str());

            if (LK == dlMain)
                Decl->ShowDef(false, S);
            else if (LK == dlMainImpl)
                Decl->ShowDef(true, S);
            else if (LK == dlA6) {
                TTypeDef *TD = 0;
                if (Decl->InheritsFrom(__classid(TTypeDecl))) {
                    TD = GetTypeDef(static_cast<TTypeDecl *>(Decl)->hDef);
                    if (TD && TD->InheritsFrom(__classid(TTemplateArgDef)))
                        Decl->ShowName(S);
                    else
                        TD = 0;
                }
                if (!TD) Decl->Show(S); // Just in case
            } else {
                // Class
                if (LK == dlClass) {
                    TList *SFieldsList     = FieldsList;
                    TList *SPropertiesList = PropertiesList;
                    TList *SMethodsList    = MethodsList;
                    TList *SArgsList       = ArgsList;
                    FieldsList             = 0;
                    PropertiesList         = 0;
                    MethodsList            = 0;
                    ArgsList               = 0;

                    if (Decl->ClassNameIs("TLocalDecl"))
                        FieldsList = SFieldsList;
                    else if (Decl->ClassNameIs("TPropDecl"))
                        PropertiesList = SPropertiesList;
                    else if (Decl->ClassNameIs("TMethodDecl"))
                        MethodsList = SMethodsList;

                    Decl->Show(S);

                    FieldsList = SFieldsList;

                    PropertiesList = SPropertiesList;
                    MethodsList    = SMethodsList;
                    ArgsList       = SArgsList;
                } else if (LK == dlArgsT) {
                    // Interface
                    TList *SFieldsList  = FieldsList;
                    TList *SMethodsList = MethodsList;
                    TList *SArgsList    = ArgsList;
                    TList *SLocalsList  = LocalsList;

                    FieldsList  = 0;
                    MethodsList = 0;
                    ArgsList    = 0;
                    LocalsList  = 0;

                    Decl->Show(S);

                    FieldsList = SFieldsList;
                    MethodsList = SMethodsList;
                    ArgsList    = SArgsList;
                    LocalsList  = SLocalsList;
                } else if (LK == dlFields) {
                    Decl->Show(S);
                } else if (LK == dlEmbedded) {
                    TList *SConstList = ConstList;
                    TList *STypeList  = TypeList;
                    ConstList         = 0;
                    TypeList          = 0;
                    Decl->Show(S);
                    ConstList = SConstList;
                    TypeList  = STypeList;
                } else
                    Decl->Show(S);
            }
            DeclCnt++;
            OutS += S;
        }
        Decl = Decl->Next;
        // Decl = static_cast<TNameDecl *>(Decl->Next);
    }
}
//------------------------------------------------------------------------------
void VisitDeclList(TDCURecVisitor *Visitor, Byte LK, TDCURec *MainRec, TDCURec *Decl) {
    PName NP;
    PName PrevNP;
    TTypeDef *TD;

    TDCURec *MainRec0 = CurMainRec;
    CurMainRec = MainRec;
    TDCURec *CurDecList0 = Decl;
    CurDecList = Decl;


    try {
        while (Decl) {
            bool Visible = Decl->IsVisible(LK);
            if (Visible)
                Visitor->doVisit(Decl);
            Decl = Decl->Next;
        }
    } __finally {
        CurDecList = CurDecList0;
        CurMainRec = MainRec0;
    }
}
//------------------------------------------------------------------------------
String __fastcall ShowRefOfsQualifier(int hDef, int Ofs) {
    TTypeDef *TD = GetGlobalTypeDef(hDef);
    if (!TD) {
        if (Ofs > 0) return Sysutils::Format("^+%d", ARRAYOFCONST((Ofs)));
        if (Ofs < 0) return Sysutils::Format("^%d", ARRAYOFCONST((Ofs)));
        return "";
    }
    return TD->GetRefOfsQualifier(Ofs);
}
//------------------------------------------------------------------------------
String __fastcall ShowTypeDef(int hDef, PName N) {
    String Result = "";

    TBaseDef *D = GetTypeDef(hDef);

    if (D)
        D->ShowNamed(N, Result);
    else {
        Result = "?";
        OutLog1("?");
    }
    return Result;
}
//------------------------------------------------------------------------------
String __fastcall ShowTypeName(int hDef) {
    String S = "";

    if (hDef <= 0 || hDef > FTypes->Count) return S;
    TBaseDef *D = (TBaseDef *) FTypes->Items[hDef - 1];
    if (!D) return S;
    PName N = D->FName;
    // was: if (!N || !N->Len) return S;
    // new:
    if (!N || N->IsEmpty()) return S;
    D->ShowName(S);
    return S;
}
//------------------------------------------------------------------------------
// Version   fxStart fxEnd   fxAdr   fxJmp   fxS fxData
// verD7     6       7       1       2       5   3
// verD2006  C       D       1       2       5   3
// verD2009  C       D       1       2       5   3
// verD2010  0       1       4       18      5   8

/**
 *
 * @param DP File 0 address, show file offsets if present
 * @param DPFile0 Dump address
 * @param FileSize Used to calculate display offset digits
 * @param SizeDispl
 * @param Size Dump size
 * @param Ofs0Displ Initial display offset
 * @param Ofs0 Offset in DCU data block - for fixups
 * @param WMin Minimal dump width (in bytes)
 * @param FixCnt
 * @param FixTbl
 */
void __fastcall ShowDump(Byte *DP, Byte *DPFile0, DWord FileSize, DWord SizeDispl, DWord Size, DWord Ofs0Displ,
                         DWord Ofs0, DWord WMin, int FixCnt, TFixupRec *FixTbl) {
    DWord LSz, dOfs, ROfs;
    Byte K, B;

    PAnsiChar DSP;
    PAnsiChar CP;
    AnsiString DS;

    if (static_cast<int>(Size) <= 0) return;

    // todo: (ShowFileOfs)
    if (DPFile0)
        ROfs = DP - DPFile0;
    else
        ROfs = DP - FMemPtr;

    Byte *LP = DP; // TIncPtr

    if (pDumpOffset)
        *pDumpOffset = ROfs;
    if (pDumpSize)
        *pDumpSize = Size;

    OutLog3("{Ofs:%lX Sz:%lX}\n", ROfs, Size);
    for (int n = 0; n < Size; n++) {
        if (n) OutLog1(" ");
        OutLog2("%02X", *(LP + n));
    }

    OutLog1("\n");

    DWord W = 16;
    if (Size < W) {
        W = Size;
        if (W < WMin) W = WMin;
    }

    // if (WMin > 0) {
    //     DumpFmt = '|%-'+IntToStr(3*W-1)+'s|';
    // else
    //     DumpFmt = '|%s|';

    PFixupRec FP = FixTbl;
    if (!FP) FixCnt = 0; // Just in case

    String Name;
    PFIXUPINFO finfo;

    do {
        LSz = W;
        if (LSz > Size) LSz = Size;
        // DS := {$IFDEF UNICODE}AnsiStrings.{$ENDIF}Format(DumpFmt{'|%s|'},[DumpStr(LP^,LSz)]);
        // DSP = PAnsiChar(DS);

        while (FixCnt > 0) {
            dOfs = (FP->OfsF & FixOfsMask - Ofs0);
            K    = reinterpret_cast<Byte *>(&FP->OfsF)[3];
            if (dOfs >= LSz && !(dOfs == LSz && K == fxEnd)) break;

            // CP = DSP + dOfs * 3;
            // switch (*CP) {
            //     case '|': *CP = '['; break;
            //     case ' ': *CP = '('; break;
            //     case '(':
            //     case '[': *CP = '{'; break;
            // }
            // if (FixUpNames) {
            //     // FS := {$IFDEF UNICODE}AnsiStrings.{$ENDIF}Format('&K%x %s',[K,CurUnit.GetAddrStr(FP^.NDX,true)]);
            //     if (FixS) FixS = FS;
            //     else FixS = {$IFDEF UNICODE}AnsiStrings.{$ENDIF}Format('%s, %s',[FixS,FS]);
            // }
            // FixCnt--;
            // FP++;

            // For IDR:
            if (FVer == verD2010 || FVer == verDXE1 || FVer == verDXE2) {
                if (K == 4 || K == 5 || K == 6 || K == 8 || K == 18) {
                    finfo = new FIXUPINFO;
                    finfo->Ofs = LP - DP + dOfs;
                    OutLog2("%lX: ", LP - DPFile0 + dOfs);
                    if (K == 4) {
                        finfo->Type = 'A';
                        OutLog1("A");
                    } else if (K == 5) {
                        finfo->Type = 'J';
                        OutLog1("J");
                    } else if (K == 6) {
                        finfo->Type = 'D';
                        OutLog1("D");
                    } else if (K == 8) {
                        finfo->Type = 'S';
                        OutLog1("S");
                    } else if (K == 18) {
                        // ???
                        finfo->Type = 'B';
                        OutLog1("B");
                    } else {
                        finfo->Type = 'U';
                        OutLog1("U");
                    }
                    Name = GetAddrStr(FP->Ndx, false);
                    finfo->Name = Name;
                    if (FixupsList) FixupsList->Add(static_cast<void *>(finfo));
                    OutLog2(" %s\n", Name.c_str());
                }
            } else if (FVer == verD7 || FVer == verD2006 || FVer == verD2009) {
                if (K == 1 || K == 2 || K == 3 || K == 5) {
                    finfo = new FIXUPINFO;
                    finfo->Ofs = LP - DP + dOfs;
                    OutLog2("%lX: ", LP - DPFile0 + dOfs);
                    if (K == 1) {
                        finfo->Type = 'A';
                        OutLog1("A");
                    } else if (K == 2) {
                        finfo->Type = 'J';
                        OutLog1("J");
                    } else if (K == 3) {
                        finfo->Type = 'D';
                        OutLog1("D");
                    } else if (K == 5) {
                        finfo->Type = 'S';
                        OutLog1("S");
                    } else {
                        finfo->Type = 'U';
                        OutLog1("U");
                    }
                    Name = GetAddrStr(FP->Ndx, false);
                    finfo->Name = Name;
                    if (FixupsList) FixupsList->Add(static_cast<void *>(finfo));
                    OutLog2(" %s\n", Name.c_str());
                }
            } else {
                if (K == 1 || K == 2 || (K == 3 && FVer != verD2)) {
                    finfo = new FIXUPINFO;
                    finfo->Ofs = LP - DP + dOfs;
                    OutLog2("%lX: ", LP - DPFile0 + dOfs);
                    if (K == 1) {
                        finfo->Type = 'A';
                        OutLog1("A");
                    } else if (K == 2) {
                        finfo->Type = 'J';
                        OutLog1("J");
                    } else if (K == 3) {
                        finfo->Type = 'D';
                        OutLog1("D");
                    } else if (K == 5) {
                        finfo->Type = 'S';
                        OutLog1("S");
                    } else {
                        finfo->Type = 'U';
                        OutLog1("U");
                    }
                    Name = GetAddrStr(FP->Ndx, false);
                    finfo->Name = Name;
                    if (FixupsList) FixupsList->Add(static_cast<void *>(finfo));
                    OutLog2(" %s\n", Name.c_str());
                }
            }
            FixCnt--;
            FP++;
        }

        Ofs0 += LSz;
        Size -= LSz;
        LP += LSz;
    } while (Size > 0);
}
//------------------------------------------------------------------------------
/**
 * Get the physical address of the block
 * @param BlOfs
 * @param BlSz
 * @param ResSz
 * @return Pointer
 */
Byte *__fastcall GetBlockMem(DWord BlOfs, DWord BlSz, DWord *ResSz) {
    *ResSz = BlSz;
    if (!FDataBlPtr || static_cast<int>(BlOfs) < 0 || !BlSz) return NULL;
    if (BlSz + BlOfs > FDataBlSize) {
        BlSz = FDataBlSize - BlOfs;
        if (static_cast<int>(BlSz) <= 0) return NULL;
    }
    *ResSz = BlSz;
    return FDataBlPtr + BlOfs;
}
//------------------------------------------------------------------------------
// todo:
void __fastcall LoadAddrToSegInfo() {
    TNDX V = ReadUIndex();

    for (int i = 1; i <= V; i++) {
        int hAddr = ReadUIndex();
        TDCURec *DR = GetAddrDef(hAddr);
        int hSeg = ReadUIndex();
        TSegKind SegKind = TSegKind::None;

        if (hSeg < 0 || hSeg > FSegCnt || FSegKindTbl == nullptr) {
            SegKind = TSegKind::None;
        } else {
            SegKind = *FSegKindTbl[hSeg];
            DR->SetSegKind(SegKind);
        }

        ReadByte();
        ReadUIndex();
        int Fix0 = ReadUIndex();
        int Size = ReadUIndex();
        ReadUIndex();
    }
}
//------------------------------------------------------------------------------
/**
 * Obtain the index of the first fixup
 * @param Ofs
 * @return
 */
int __fastcall GetStartFixup(DWord Ofs) {
    if (!FFixupTbl || !FFixupCnt) return 0;
    if (!Ofs) return 0;
    int iMin = 0;
    int iMax = FFixupCnt - 1;
    while (iMin <= iMax) {
        int i = (iMin + iMax) / 2;
        TFixupRec fRec = FFixupTbl[i];
        int d = fRec.OfsF & FixOfsMask - Ofs;
        if (d < 0)
            iMin = i + 1;
        else
            iMax = i - 1;
    }
    return iMin;
}
//------------------------------------------------------------------------------
void __fastcall ShowDataBl(DWord Ofs0, DWord BlOfs, DWord BlSz) {
    Byte *DP = GetBlockMem(BlOfs + Ofs0, BlSz - Ofs0, &BlSz);
    if (!DP) return;
    int Fix0 = GetStartFixup(BlOfs + Ofs0);
    ShowDump(DP, FMemPtr, FMemSize, 0, BlSz, Ofs0, BlOfs + Ofs0, 0, FFixupCnt - Fix0, &FFixupTbl[Fix0]);
}
//------------------------------------------------------------------------------
/**
 *
 * @param DP Pointer
 * @param DS
 * @param Ofs0
 */
void __fastcall ShowDataBlP(Byte *DP, DWord DS, DWord Ofs0) {
    if ((DP >= FDataBlPtr) && (DP < FDataBlPtr + FDataBlSize)) {
        ShowDataBl(Ofs0, DP - FDataBlPtr, DS);
    } else {
        ShowDump(DP, FMemPtr, FMemSize, 0, DS, Ofs0, Ofs0, 0, 0, nullptr);
    }
}
//------------------------------------------------------------------------------
// todo?: DasmCodeBlSeq(Ofs0,BlOfs,BlSz,SzMax: Cardinal; Seq: TCmdSeq; WasPartMsg: Boolean;
void __fastcall DasmCodeBlSeq(DWord Ofs0, DWord BlOfs, DWord BlSz) {
    // todo? TCmd C;
    Byte *DP = GetBlockMem(BlOfs, BlSz, &BlSz);
    if (!DP) {
        printf("Warning: nodump\n");
        return;
    }
    int   Fix0  = GetStartFixup(BlOfs);
    char *FOfs0 = reinterpret_cast<char *>(FMemPtr);
    ShowDump(DP, reinterpret_cast<Byte *>(FOfs0), FMemSize, BlSz, BlSz, Ofs0, BlOfs, 0, FFixupCnt - Fix0, &FFixupTbl[Fix0]);
}
//------------------------------------------------------------------------------
/**
 *
* @param Ofs0 Virtual address of the block start (>0 only for procedures, which where linked from some .obj file;
*             there could be near jumps or conditional jumps between such procedures)
 * @param BlOfs Offset of the block in the unit data block
 * @param BlSz Size of the block.
 */
void __fastcall ShowCodeBl(DWord Ofs0, DWord BlOfs, DWord BlSz) {
    DWord CodeSz = BlSz;
    DasmCodeBlSeq(Ofs0, BlOfs, CodeSz);

    if (CodeSz < BlSz) {
        OutLog1("rest:\n");
        ShowDataBl(Ofs0 + CodeSz, BlOfs + CodeSz, Ofs0 + BlSz);
    }
}
//------------------------------------------------------------------------------
bool __fastcall MemToUInt(Byte *DP, DWord Sz, DWord *Res) {
    switch (Sz) {
        case 1: *Res = *((Byte *) DP); break;
        case 2: *Res = *reinterpret_cast<Word *>(DP); break;
        case 4: *Res = *reinterpret_cast<DWord *>(DP); break;
        default: Res = nullptr; return false;
    }
    return true;
}
//------------------------------------------------------------------------------
AnsiString __fastcall CharStr(char Ch) {
    char buf[256];
    if (Ch < ' ')
        sprintf(buf, "#%d", static_cast<Byte>(Ch));
    else
        sprintf(buf, "'%c'", Ch);
    return String(buf);
}
//------------------------------------------------------------------------------
AnsiString __fastcall StrConstStr(char *CP, int L) { // was String
    bool WasCode;
    char Ch;

    AnsiString Result;
    Result.SetLength(3 * L + 2);
    int  LRes = 0;
    bool Code = true;
    while (L > 0) {
        Ch = *CP;
        CP++;
        L--;
        WasCode = Code;
        Code    = (Ch < ' ');
        if (WasCode != Code) {
            LRes++;
            Result[LRes] = '\'';
        }
        if (Code) {
            AnsiString S = CharStr(Ch);
            Move(&S[1], &Result[LRes + 1], S.Length());
            LRes += S.Length();
        } else {
            if (Ch == '\'') {
                LRes++;
                Result[LRes] = '\'';
            }
            LRes++;
            Result[LRes] = Ch;
        }
    }
    if (!Code) {
        LRes++;
        Result[LRes] = '\'';
    }
    if (!LRes)
        Result = "''";
    else
        Result.SetLength(LRes);
    return Result;
}
//------------------------------------------------------------------------------
int __fastcall ShowStrConst(Byte *DP, DWord DS, String &OutS) {
    int    L;
    String Value;
    OutS = "";

    if (DS < 9) return -1;
    if ((*reinterpret_cast<int *>(DP)) != -1) return -1;
    Byte *VP = DP + sizeof(int);
    L = (*reinterpret_cast<int *>(VP));
    if (DS < L + 9) return -1;
    VP += sizeof(int);
    if (*(VP + L)) return -1;
    Value = StrConstStr(reinterpret_cast<char *>(VP), L);
    OutS  = Value;
    OutLog2("%s", Value.c_str());
    return L + 9;
}
//------------------------------------------------------------------------------
// The Unicode string support for ver>=verD12
// New string header:
//  -12:2 - Code page
//  -10:2 - string item size
//  -8:4 - reference count
//  -4:4 - Length in terms of the string item size
// Ver >=verD12
int __fastcall ShowUnicodeStrConst(Byte *DP, DWord DS, String &OutS) {
    int Result = -1;

    OutS = "";
    if (DS < 13) return Result;
    Byte *VP   = DP + sizeof(Word);
    int   ElSz = *reinterpret_cast<Word *>(VP);
    if (ElSz == sizeof(AnsiChar)) {
        // Try to show it as an AnsiString (!!!the code page is ignored by now)
        Result = ShowStrConst(DP + 2 * sizeof(Word), DS - 2 * sizeof(Word), OutS);
        if (Result > 0) Result += 2 * sizeof(Word);
        return Result;
    }
    if (ElSz != sizeof(WideChar)) return Result;
    VP = DP + sizeof(int);
    if (*reinterpret_cast<int *>(VP) != -1) return Result;
    VP += sizeof(int);
    Result = ShowUnicodeResStrConst(VP, DS - 2 * sizeof(int), OutS);
    if (Result < 0) return Result;
    Result += 2 * sizeof(int);
    return Result;
}
//------------------------------------------------------------------------------
// Ver >=verD12
int __fastcall ShowUnicodeResStrConst(Byte *DP, DWord DS, String &OutS) {
    int Result = -1;

    OutS  = "";
    int L = *reinterpret_cast<int *>(DP);
    if (DS - 6 < L * sizeof(WideChar) || L < 0) return Result;
    DP += sizeof(int);
    if (*reinterpret_cast<PWideChar>(DP + L * sizeof(WideChar)) != 0) return Result;

    WideString wide = WideString(reinterpret_cast<PWideChar>(DP), L);
    String     str  = wide;

    Result = L * sizeof(WideChar) + 6;
    OutS   = StrConstStr(AnsiString(str).c_str(), L);

    OutLog2("%s", OutS.c_str());
}
//------------------------------------------------------------------------------
/**
 * TUnit.ShowTypeValue
 *
 * @param T TypeDef
 * @param DP
 * @param DS
 * @param ConstKind
 * @param IsNamed
 * @param OutS
 * @return Size used
 */
int __fastcall ShowTypeValue(TTypeDef *T, Byte *DP, DWord DS, int ConstKind, bool IsNamed, String &OutS) {
    int Result;
    TFixupMemState MS;
    OutS = "";
    Extended E;

    if (!T) return -1;
    if (ConstKind >= 0) {
        SaveFixupMemState(&MS);
        SetCodeRange(DP, DP, DS);
    }

    if (ConstKind >= 0 && T->InheritsFrom(__classid(TStringDef))) {
        if (FVer >= verD2009 && FVer < verK1) {
            if (ConstKind == 2) {
                Result = ShowUnicodeResStrConst(DP, DS, OutS);
            } else {
                Result = ShowUnicodeStrConst(DP, DS, OutS);
            }
        } else {
            if (ConstKind == 2) {
                if (IsNamed) {
                    // todo:PutS('WideString'); //!!!ToDo: add some parameter, that the typecast
                    // is required (called from const and not from parameter default value)
                }
                Result = ShowUnicodeResStrConst(DP, DS, OutS);
                if (IsNamed) {
                    // todo: PutCh(')');
                }
            } else {
                Result = ShowStrConst(DP, DS, OutS);
            }
        }
    } else if ((ConstKind == 3) && T->InheritsFrom(__classid(TFloatDef)) && (DS == sizeof(Extended))) {
        E = static_cast<Extended>(*DP);
        if (static_cast<TFloatDef *>(T)->Kind == fkCurrency) {
            E = E * 0.0001;
        }
        // todo: PutS(FixFloatToStr(E,TFloatDef(T).Kind=fkComp{NeedDot})); //PutsFmt('%g',[E]); starting from D7 writes 3 digits after E
        Result = sizeof(Extended);
    } else {
        Result = T->ShowValue(DP, DS, OutS);
    }

    if (ConstKind >= 0) RestoreFixupMemState(&MS);
    return Result;
}
//------------------------------------------------------------------------------
int __fastcall ShowGlobalTypeValue(int hDef, Byte *DP, DWord DS, bool AndRest, int ConstKind, bool IsNamed, String &OutS) {
    OutS = "";
    if (!DP) return -1;

    TTypeDef *T = GetGlobalTypeDef(hDef);

    int Result = ShowTypeValue(T, DP, DS, ConstKind, IsNamed, OutS); // IsNamed

    if (!AndRest) return Result;
    int SzShown = Result;
    if (SzShown < 0) SzShown = 0;
    if (SzShown >= DS) return Result;

    if (DP >= FDataBlPtr && DP < FDataBlPtr + FDataBlSize)
        ShowDataBl(SzShown, DP - FDataBlPtr, DS);
    else {
        Byte *FOfs0;
        FOfs0 = FMemPtr;
        ShowDump(DP, FOfs0, FMemSize, 0, DS, SzShown, SzShown, 0, 0, NULL);
    }
    return Result;
}
//------------------------------------------------------------------------------
TDCURec *__fastcall GetGlobalAddrDef(int hDef) {
    TDCURec *D = GetAddrDef(hDef);
    if (!D) return NULL;
    if (D->InheritsFrom(__classid(TImpDef))) return NULL;
    return D;
}
//------------------------------------------------------------------------------
TTypeDef *__fastcall GetLastAddedTypeDef() {
    if (FTypeDefCnt > 0 && FTypeDefCnt <= FTypes->Count)
        return (TTypeDef *) FTypes->Items[FTypeDefCnt - 1];

    return nullptr;
}
//------------------------------------------------------------------------------
bool __fastcall ShowGlobalConstValue(int hDef, String &OutS) {
    TDCURec *D = GetGlobalAddrDef(hDef);
    if (!D || !D->InheritsFrom(__classid(TConstDecl))) return false;
    static_cast<TConstDecl *>(D)->ShowValue(OutS);
    return true;
}
//------------------------------------------------------------------------------
String __fastcall ShowOfsQualifier(int hDef, int Ofs) {
    TTypeDef *TD = GetGlobalTypeDef(hDef);
    if (!TD) {
        if (Ofs > 0)
            return Sysutils::Format("+%d", ARRAYOFCONST((Ofs)));
        else if (Ofs < 0)
            return Sysutils::Format("%d", ARRAYOFCONST((Ofs)));
        return "";
    }
    return TD->GetOfsQualifier(Ofs);
}
//------------------------------------------------------------------------------
/**
 * Scan One DCU
 * References dcu32int's TUnit.DecodeMagic() and TUnit.Load()
 * @param Filename
 * @return boolean status
 */
bool __fastcall ScanOneDCU(String Filename) {
    Byte   B;
    String S;

    FFName = Filename;
    FFExt  = ExtractFileExt(FFName);

    FILE *fIn = fopen(AnsiString(Filename).c_str(), "rb");
    if (!fIn) return false;

    ModuleInfo = new MODULEINFO;
    ModuleInfo->ModuleID = ModuleList->Count;
    ModuleInfo->UsesList = new TStringList;
    ModuleList->Add(static_cast<void *>(ModuleInfo));
    ModuleID = ModuleInfo->ModuleID;
    ModuleInfo->Filename = Filename;

    FUnitImp->Clear();
    FTypes->Clear();
    FAddrs->Clear();
    FTypeShowStack->Clear();

    GenVarCAsVars = false;
    FVer = 0;
    fxStart = fxStart30;
    fxEnd = fxEnd30;
    DefStart = nullptr;
    FExportNames = 0;
    FDecls = nullptr;
    FTypeDefCnt = 0;
    FDataBlPtr = nullptr;
    FFixupTbl = nullptr;
    FLocVarTbl = nullptr;
    FEmbeddedLists = nullptr;

    fseek(fIn, 0, SEEK_END);
    FMemSize = ftell(fIn);
    fseek(fIn, 0, SEEK_SET);
    FMemPtr = new Byte[FMemSize];
    fread(static_cast<void *>(FMemPtr), 1, FMemSize, fIn);
    fclose(fIn);

    CurPos = FMemPtr;
    IsMSIL = false;

    TScanState CP0;
    ChangeScanState(CP0, FMemPtr, FMemSize);

    try {
        // Read Magic
        Magic = ReadULong();
        FPtrSize = 4;
        fxJmpAddr = fxJmpAddr0;

        switch (Magic) {
            case 0x50505348:
                FVer    = verD2;
                fxStart = fxStart20;
                fxEnd   = fxEnd20;
                break;
            case 0x44518641:
                FVer    = verD3;
                fxStart = fxStart30;
                fxEnd   = fxEnd30;
                break;
            case 0x4768A6D8:
                FVer    = verD4;
                fxStart = fxStart30;
                fxEnd   = fxEnd30;
                break;
            case 0xF21F148B:
                FVer    = verD5;
                fxStart = fxStart30;
                fxEnd   = fxEnd30;
                break;
            case 0x0E0000DD:
            case 0x0E8000DD:
                FVer    = verD6;
                fxStart = fxStart30;
                fxEnd   = fxEnd30;
                break;
            case 0xFF0000DF:
            case 0x0F0000DF:
            case 0x0F8000DF:
                FVer    = verD7;
                fxStart = fxStart70;
                fxEnd   = fxEnd70;
                break;
            case 0x10000229:
                FVer    = verD8;
                IsMSIL  = true;
                fxStart = fxStartMSIL;
                fxEnd   = fxEndMSIL;
                break;
            case 0x11000239:
                FVer    = verD2005;
                IsMSIL  = true;
                fxStart = fxStartMSIL;
                fxEnd   = fxEndMSIL;
                break;
            case 0x1100000D:
            case 0x11800009:
                FVer    = verD2005;
                fxStart = fxStart70;
                fxEnd   = fxEnd70;
                break;
            case 0x12000023:
                FVer    = verD2006; // Delphi 2006, 2007
                fxStart = fxStart100;
                fxEnd   = fxEnd100;
                break;
            case 0x1200024D:
                FVer    = verD2006;
                IsMSIL  = true;
                fxStart = fxStartMSIL;
                fxEnd   = fxEndMSIL;
                break;
            case 0x14000039:
                FVer    = verD2009; // Delphi 2009
                fxStart = fxStart100;
                fxEnd   = fxEnd100;
                break;
            case 0x15000045:
                FVer      = verD2010; // Delphi 2010
                fxStart   = fxStart2010;
                fxEnd     = fxEnd2010;
                fxJmpAddr = fxJmpAddrXE; // Was checked for XE only
                break;
            case 0x1600034B:
                FVer      = verDXE1; // DelphiXE1
                fxStart   = fxStart2010;
                fxEnd     = fxEnd2010;
                fxJmpAddr = fxJmpAddrXE; // Was checked for XE only
                break;
            case 0xF21F148C:
                FVer    = verK1; // Kylix 1.0
                fxStart = fxStart30;
                fxEnd   = fxEnd30;
                break;
            case 0x0E1011DD:
            case 0x0E0001DD:
                FVer    = verK2; // Kylix 2.0
                fxStart = fxStart30;
                fxEnd   = fxEnd30;
                break;
            case 0x0F1001DD:
            case 0x0F0001DD:
                FVer = verK3; // Kylix 3.0
                break;
            default:
                // All the other versions follow the common scheme of magic values assignment,
                // which we describe here:
                if ((Magic & 0x00FF00F9) == 0x49) {
                    DWord BVer    = Magic >> 24;
                    DWord PlMagic = Magic & 0xFF;
                    if ((BVer <= 0x24 && BVer >= 0x1B && PlMagic == 0x4D) ||
                        (BVer <= 0x1A && BVer >= 0x17 && PlMagic == 0x4B)) {
                        PlMagic   = (Magic >> 8) & 0xFF;
                        FVer      = BVer + (verDXE2 - 0x17);
                        fxJmpAddr = fxJmpAddrXE;

                        printf("Delphi version: %d\n", FVer);

                        switch (PlMagic) {
                            case 0x03:
                                FPlatform = dcuplWin32;
                                fxStart   = fxStart2010;
                                fxEnd     = fxEnd2010;
                                break;
                            case 0x23:
                                FPlatform = dcuplWin64;
                                FPtrSize  = 8;
                                break;
                            case 0x04: FPlatform = dcuplOsx32; break;
                            case 0x24: // OSX 64 support was added in 10.4 Sydney
                                if (FVer >= verD10_4) {
                                    FPlatform = dcuplOsx64;
                                    FPtrSize  = 8;
                                }
                                break;
                            case 0x84: // OSX Arm 64 support was added in 12 Athens
                                if (FVer >= verD12) {
                                    FPlatform = dcuplOsxArm64;
                                    FPtrSize  = 8;
                                }
                                break;
                            case 0x14: // iOS support was added in XE4
                                if (FVer >= verDXE4) FPlatform = dcuplIOSEmulator;
                                break;
                            case 0x88: // iOS Arm 64 Simulator support was added in 12 Athens
                                if (FVer >= verD12) {
                                    FPlatform = dcuplIOSSimArm64;
                                    FPtrSize  = 8;
                                }
                                break;
                            case 0x76: // iOS support was added in XE4
                                if (FVer >= verDXE4) FPlatform = dcuplIOSDevice;
                                break;
                            case 0x86: // iOS64 code was changed in Delphi 11
                                if (FVer >= verD11) {
                                    FPlatform = dcuplIOSDevice64;
                                    FPtrSize  = 8;
                                }
                                break;
                            case 0x94: // iOS64 support was added in XE8, and the code was changed in Delphi 11
                                if (!(FVer < verDXE8 || FVer >= verD11)) {
                                    FPlatform = dcuplIOSDevice64;
                                    FPtrSize  = 8;
                                }
                                break;
                            case 0x67: // Android code was changed in Delphi 10.4
                                if (FVer >= verD10_4) FPlatform = dcuplAndroid;
                                break;
                            case 0x77: // Android support was added in XE4 and the code was changed in Delphi 10.4
                                if (!(FVer < verDXE5 || FVer >= verD10_4)) FPlatform = dcuplAndroid;
                                break;
                            case 0x87: // Android 64 support was added in 10.4 Sydney
                                if (FVer >= verD10_4) {
                                    FPlatform = dcuplAndroid64;
                                    FPtrSize  = 8;
                                }
                                break;
                            case 0x21: // Linux support was added in XE 10.2
                                if (FVer >= verD10_2) {
                                    FPlatform = dcuplLinux64;
                                    FPtrSize  = 8;
                                }
                                break;
                            default:
                                printf("[Error] Unable to decode magic value %lX\n", Magic);
                                return false;
                                break;
                        }
                        }
                } else {
                    printf("Error: Wrong magic %lX\n", Magic);
                    delete[] FMemPtr;
                    return false;
                }
                break;
        }

        if (FVer >= 100) {
            IsKylix = true;
        } else {
            IsDelphi = true;
        }

        printf("Magic: %lX\n", Magic);

        // TUnit.SetupFixups
        fx8Byte = false;
        int i;
        if (fxStart > 0) {
            fxValid = System::Set<std::uint8_t, 0, fxMax>();
            if (fxStart - 1 >= 0) {
                fxValid << 0 << (fxStart - 1);
            }
            for (i = 0; i < fxStart; ++i) {
                fxSize[i] = 4;
            }
            for (i = fxStart; i <= fxMax; ++i) {
                fxSize[i] = -1;
            }
        } else if (FVer < verDXE2 || FPlatform != dcuplWin64) {
            fxValid = System::Set<std::uint8_t, 0, fxMax>();
            if (fxEnd + 1 <= fxMaxXE) {
                fxValid << (fxEnd + 1) << fxMaxXE;
            }
            std::memcpy(fxSize, fxSizeXE32, sizeof(TFxSizeTbl));
        } else {
            // Build set [fxEnd+1..fxMax]
            fxValid = System::Set<std::uint8_t, 0, fxMax>();
            if (fxEnd + 1 <= fxMax) {
                fxValid << (fxEnd + 1) << fxMax;
            }
            std::memcpy(fxSize, fxSizeXE64, sizeof(TFxSizeTbl));
            fx8Byte = true;
        }

        // From TUnit.ReadUnitHeader:

        // Read File Header
        FileSizeH = ReadULong();
        if (FileSizeH != FMemSize) {
            printf("[Error] Wrong size: %lX != %lX\n", FileSizeH, FMemSize);
            delete[] FMemPtr;
            return false;
        }

        // Read FileTime
        FT = ReadULong();

        if (FVer == verD2) {
            B = ReadByte();
            Tag = ReadTag();
        } else {
            Stamp = ReadULong();
            B = ReadByte();
            if (FVer >= verD7 && FVer < verK1) {
                B = ReadByte(); // It has another header byte (or index)
                AddAddrDef(nullptr); // Self-reference added
            }
            if (FVer >= verD2005 && FVer < verK1) {
                AnsiString sName = ReadStr();
                printf("Debug: Unit name: %s\n", sName.c_str());
            }
            if (FVer >= verD2009 && FVer < verK1) {
                DWord L1 = ReadUIndex();
                DWord L2 = ReadUIndex();
            }
            Tag = ReadTag();
            if (FVer >= verK1) {
                if (Tag == drUnit4) {
                    do {
                        DWord L = ReadULong();
                        Tag = ReadTag();
                    } while (Tag != drUnit4);
                } else if (Tag != drUnitFlags) {
                    SkipBlock(3);
                    Tag = ReadTag();
                }
            }
            if (Tag == drUnitFlags) {
                Flags = ReadUIndex();
                if (FVer > verD2005 && FVer < verK1)
                    DWord Flags1 = ReadUIndex();
                if (FVer > verD3)
                    UnitPrior = ReadUIndex();
                Tag = ReadTag();
            }
            if (Tag == drInDcpWin64Info) {
                if (FVer >= verD11 && FVer < verK1 && FromPackage) {
                    ReadInDcpWin64Info();
                    Tag = ReadTag();
                }
            }
        }

        // From TUnit.Load:
        ReadSourceFiles();
        ReadUses(drUnit);
        ReadUses(drUnit1);
        ReadUses(drDLL);

        if (FVer >= verD12 && (FPlatform == dcuplIOSSimArm64 || FPlatform == dcuplIOSDevice64))
            ReadUses(drDLLInfo1);

        try {
            ReadDeclList(dlMain, nullptr, &FDecls);

            // Let's ignore unknown tags after drCBlock and drFixUp, but not before
            if (!(FPlatform == dcuplOsx64 && FPlatform == dcuplIOSDevice && FPlatform == dcuplIOSDevice64 &&
                FPlatform == dcuplIOSSimArm64 && FPlatform == dcuplOsxArm64 && FPlatform == dcuplAndroid &&
                FPlatform == dcuplAndroid64 && FPlatform == dcuplLinux64) && (FDataBlPtr == nullptr || FFixupTbl == nullptr)) {
                printf("[Error] ScanOneDCU: stop tag\n"); // DCUError('stop tag');
            }
        } __finally {
            SetExportNames(FDecls); // Moved before BindEmbeddedTypes, because some embedded types (like TList<T>.TEnumerator) should be exported

            if (FVer >= verDXE1 && FVer < verK1)
                BindEmbeddedTypes(); // try to fix the local types relocation problem of XE

            SetEnumConsts(&FDecls);
            FillProcLocVarTbls();
        }
    } __finally {
        FLoaded = true;
        RestoreScanState(CP0);

    }

    // From TUnit.Show:
    // SetShowAuxValues();
    ShowSourceFiles();
    // interface
    ShowUses("uses", TUnitImpFlags::None);
    ShowDeclList(dlMain, FDecls, S);
    // implementation
    ShowUses("uses", TUnitImpFlags::Impl);
    ShowUses("imports", TUnitImpFlags::DLL);
    ModuleInfo->UsesList->Sort();
    ShowDeclList(dlMainImpl, FDecls, S);

    // From TUnit.Destroy:
    FreeDCURecList(FDecls);
    delete[] FMemPtr;
    return true;
}
//------------------------------------------------------------------------------
// Сортировка по именам
// Sort by name
int __fastcall CompareModulesByName(void *Item1, void *Item2) {
    PMODULEINFO info1 = static_cast<PMODULEINFO>(Item1);
    PMODULEINFO info2 = static_cast<PMODULEINFO>(Item2);
    return CompareText(info1->Name, info2->Name);
}
//------------------------------------------------------------------------------
// Сортировка по ModuleID
// Sort by ModuleID
int __fastcall CompareModulesByID(void *Item1, void *Item2) {
    PMODULEINFO info1 = static_cast<PMODULEINFO>(Item1);
    PMODULEINFO info2 = static_cast<PMODULEINFO>(Item2);
    if (info1->ModuleID > info2->ModuleID) return 1;
    if (info1->ModuleID < info2->ModuleID) return -1;
    return 0;
}
//------------------------------------------------------------------------------
int __fastcall CompareConstsByName(void *Item1, void *Item2) {
    PCONSTINFO info1 = static_cast<PCONSTINFO>(Item1);
    PCONSTINFO info2 = static_cast<PCONSTINFO>(Item2);
    int        res   = CompareText(info1->Name, info2->Name);
    if (res) return res;
    if (info1->ModuleID > info2->ModuleID) return 1;
    if (info1->ModuleID < info2->ModuleID) return -1;
    return 0;
}
//------------------------------------------------------------------------------
int __fastcall CompareConstsByID(void *Item1, void *Item2) {
    PCONSTINFO info1 = static_cast<PCONSTINFO>(Item1);
    PCONSTINFO info2 = static_cast<PCONSTINFO>(Item2);
    if (info1->ModuleID > info2->ModuleID) return 1;
    if (info1->ModuleID < info2->ModuleID) return -1;
    return CompareText(info1->Name, info2->Name);
}
//------------------------------------------------------------------------------
int __fastcall CompareTypesByName(void *Item1, void *Item2) {
    PTYPEINFO info1 = static_cast<PTYPEINFO>(Item1);
    PTYPEINFO info2 = static_cast<PTYPEINFO>(Item2);
    int       res   = CompareText(info1->Name, info2->Name);
    if (res) return res;
    if (info1->ModuleID > info2->ModuleID) return 1;
    if (info1->ModuleID < info2->ModuleID) return -1;
    return 0;
}
//------------------------------------------------------------------------------
int __fastcall CompareTypesByID(void *Item1, void *Item2) {
    PTYPEINFO info1 = static_cast<PTYPEINFO>(Item1);
    PTYPEINFO info2 = static_cast<PTYPEINFO>(Item2);
    if (info1->ModuleID > info2->ModuleID) return 1;
    if (info1->ModuleID < info2->ModuleID) return -1;
    return CompareText(info1->Name, info2->Name);
}
//------------------------------------------------------------------------------
int __fastcall CompareVarsByName(void *Item1, void *Item2) {
    PVARINFO info1 = static_cast<PVARINFO>(Item1);
    PVARINFO info2 = static_cast<PVARINFO>(Item2);
    int      res   = CompareText(info1->Name, info2->Name);
    if (res) return res;
    if (info1->ModuleID > info2->ModuleID) return 1;
    if (info1->ModuleID < info2->ModuleID) return -1;
    return 0;
}
//------------------------------------------------------------------------------
int __fastcall CompareVarsByID(void *Item1, void *Item2) {
    PVARINFO info1 = static_cast<PVARINFO>(Item1);
    PVARINFO info2 = static_cast<PVARINFO>(Item2);
    if (info1->ModuleID > info2->ModuleID) return 1;
    if (info1->ModuleID < info2->ModuleID) return -1;
    return CompareText(info1->Name, info2->Name);
}
//------------------------------------------------------------------------------
int __fastcall CompareResStrsByName(void *Item1, void *Item2) {
    PRESSTRINFO info1 = static_cast<PRESSTRINFO>(Item1);
    PRESSTRINFO info2 = static_cast<PRESSTRINFO>(Item2);
    int         res   = CompareText(info1->Name, info2->Name);
    if (res) return res;
    if (info1->ModuleID > info2->ModuleID) return 1;
    if (info1->ModuleID < info2->ModuleID) return -1;
    return 0;
}
//------------------------------------------------------------------------------
int __fastcall CompareResStrsByID(void *Item1, void *Item2) {
    PRESSTRINFO info1 = static_cast<PRESSTRINFO>(Item1);
    PRESSTRINFO info2 = static_cast<PRESSTRINFO>(Item2);
    if (info1->ModuleID > info2->ModuleID) return 1;
    if (info1->ModuleID < info2->ModuleID) return -1;
    return CompareText(info1->Name, info2->Name);
}
//------------------------------------------------------------------------------
int __fastcall CompareProcsByName(void *Item1, void *Item2) {
    PPROCDECLINFO info1 = static_cast<PPROCDECLINFO>(Item1);
    PPROCDECLINFO info2 = static_cast<PPROCDECLINFO>(Item2);
    int           res   = CompareText(info1->Name, info2->Name);
    if (res) return res;
    if (info1->ModuleID > info2->ModuleID) return 1;
    if (info1->ModuleID < info2->ModuleID) return -1;
    return 0;
}
//------------------------------------------------------------------------------
int __fastcall CompareProcsByID(void *Item1, void *Item2) {
    PPROCDECLINFO info1 = static_cast<PPROCDECLINFO>(Item1);
    PPROCDECLINFO info2 = static_cast<PPROCDECLINFO>(Item2);
    if (info1->ModuleID > info2->ModuleID) return 1;
    if (info1->ModuleID < info2->ModuleID) return -1;
    if (info1->ID > info2->ID) return 1;
    if (info1->ID < info2->ID) return -1;
    return 0;
}
//------------------------------------------------------------------------------
/**
 * Записываем длину имени (Word) + имя + нулевой байт, возвращаем общее кол-во байт
 * Write the length of the name (Word) + name + zero byte, return the total number of bytes
 *
 * From KB.pas
 *
 * @param fDst Destination file
 * @param str String to write
 * @return Total number of written bytes
 */
int __fastcall WriteString(FILE* fDst, const String& str) {
    int Bytes = 0;
    Byte ZeroB = 0;

    Word NameLength = static_cast<Word>(str.Length());
    if (fDst) fwrite(&NameLength, sizeof(NameLength), 1, fDst);
    if (NameLength) {
        if (fDst) fwrite(str.c_str(), 1, NameLength, fDst);
    }
    if (fDst) fwrite(&ZeroB, 1, 1, fDst);
    return sizeof(NameLength) + NameLength + 1;
}
//------------------------------------------------------------------------------
/**
 * From KB.pas
 *
 * @param fSrc Source file
 * @param fDst Destination file
 * @param SrcOffset Source offset
 * @param Bytes
 * @return
 */
int __fastcall WriteDump(FILE *fSrc, FILE *fDst, long SrcOffset, DWord Bytes) {
    fseek(fSrc, SrcOffset, SEEK_SET);
    for (int m = 0; m < Bytes; m++) {
        Byte b;
        fread(&b, 1, 1, fSrc);
        fwrite(&b, 1, 1, fDst);
    }
    return Bytes;
}
//------------------------------------------------------------------------------
/**
 * From KB.pas
 *
 * @param fDst Destination file
 * @param FixupList
 * @param Bytes
 * @param Name
 * @return
 */
int __fastcall WriteRelocs(FILE *fDst, TList *FixupList, DWord Bytes, String Name) {
    Byte  Byte00 = 0;
    DWord ByteFF = 0xFFFFFFFF;
    int   ByteNo = 0;

    for (int n = 0; n < FixupList->Count; n++) {
        PFIXUPINFO finfo = (PFIXUPINFO) FixupList->Items[n];
        // Если информация о фиксапе занимает больше места, чем нужно, не записываем ее
        // If the fixup information takes up more space than necessary, do not write it down.
        if (finfo->Ofs + 4 > Bytes) continue;
        if (finfo->Ofs < ByteNo) continue;
        // Свободное от релоков место заполняем 0
        // We fill the space free from relocations with 0
        for (ByteNo; ByteNo < finfo->Ofs; ByteNo++) fwrite(&Byte00, 1, 1, fDst);
        // Сам релок заполняем 4-мя байтами 0xFF
        // We fill the relock itself with 4 bytes 0xFF
        fwrite(&ByteFF, 1, 4, fDst);
        ByteNo += 4;
    }
    // Оставшиеся байты заполняем 0
    // Fill the remaining bytes with 0
    for (ByteNo; ByteNo < Bytes; ByteNo++) fwrite(&Byte00, 1, 1, fDst);

    return Bytes;
}
//------------------------------------------------------------------------------
/**
 * From KB.pas
 *
 * @param fDst Destination file
 * @param FixupList
 * @return
 */
int __fastcall WriteFixups(FILE *fDst, TList *FixupList) {
    int Bytes = 0;
    for (int m = 0; m < FixupList->Count; m++) {
        PFIXUPINFO finfo = (PFIXUPINFO) FixupList->Items[m];
        // write Type
        if (fDst) fwrite(&finfo->Type, sizeof(finfo->Type), 1, fDst);
        Bytes += sizeof(finfo->Type);
        // write Offset
        if (fDst) fwrite(&finfo->Ofs, sizeof(finfo->Ofs), 1, fDst);
        Bytes += sizeof(finfo->Ofs);
        // write Name
        Bytes += WriteString(fDst, finfo->Name);
    }
    return Bytes;
}
//------------------------------------------------------------------------------
void VisitDecls(TDCURecVisitor *Visitor, bool InterfaceOnly) {
    TDCURec *Decl = FDecls;
    while (Decl) {
        // InterfaceOnly is false by default
        if (!InterfaceOnly || Decl->IsVisible(dlMain)) {
            Visitor->doVisit(Decl);
        }
        Decl = Decl->Next;
    }
}
//------------------------------------------------------------------------------
void VisitTypes(TDCURecVisitor *Visitor) {
    for (int i = 0; i < FTypes->Count; i++) {
        TBaseDef *D = (TBaseDef *) FTypes->Items[i];
        if (D) Visitor->doVisit(D);
    }
}

//------------------------------------------------------------------------------

void __fastcall ChangeScanState(TScanState State, Byte *DP, DWord MaxSz) {
    State         = ScSt;
    ScSt.StartPos = DP;
    ScSt.CurPos   = DP;
    ScSt.EndPos   = DP + MaxSz;
}

void RestoreScanState(TScanState State) {
    ScSt = State;
}

//------------------------------------------------------------------------------
#pragma argsused
// int main(int argc, char* argv[])

int _tmain(int argc, _TCHAR *argv[]) {
    printf("Knowledge Base Builder for IDR by crypto and Alexei Hmelnov\n\n");

    InputParser input(argc, argv);

    if (input.cmdOptionExists("-h") || argc == 1) {
        // Show usage
        printf("Usage:\n\tBuildKB.exe KBName <DCUList>\n");
        // todo: proposed flags/options:
        // -f = single file
        // -d = directory of files
        // -x = pattern match files
        // -o = kb output file
        // -l = logfile output path
        // -v = version
        return -1;
    }

    int   num         = 0;
    FILE *fList       = 0;
    // int   iAttributes = faReadOnly | faArchive;
    int   iAttributes = faAnyFile; //  | ~faDirectory;
    char  dcuFilename[256];

    TSearchRec sr;

    String dirPath;
    String searchPattern;

    /*if (argc != 2 && argc != 3) {
        printf("Usage:\n BuildKB.exe KBName <DCUList>\n");
        return -1;
    }*/

    /*fOut = fopen(argv[1], "wb+");
    if (!fOut) {
        printf("[Error] Cannot open KB file %s\n", argv[1]);
        return -1;
    }*/

    // todo:
    /*const String &logFile = input.getCmdOption("-l");
    if (!logFile.IsEmpty()) {
        fLog = fopen(AnsuStrung*logFile).c_str(), "wt+");
        if (!fLog) {
            printf("[Error] Cannot open log file %s\n", logFile.c_str());
            return -1;
        }
    } else {
        // default path
    }*/

    fLog = fopen("BuildKB.log", "wt+");

    // todo: check presence of all arguments (-o, -f/-d) before proceeding

    ModuleList = new TList;
    ConstList  = new TList;
    TypeList   = new TList;
    VarList    = new TList;
    ResStrList = new TList;
    ProcList   = new TList;

    FUnitImp       = new TList;
    FTypes         = new TList;
    FAddrs         = new TList;
    FTypeShowStack = new TList;

    // KB Output File
    // todo: append flag
    const String &output = input.getCmdOption("-o");
    if (!output.IsEmpty()) {
        fOut = fopen(AnsiString(output).c_str(), "wb+");
        if (!fOut) {
            printf("[Error] Cannot open KB file %s\n", output.c_str());
            return -1;
        }
    }

    // Process a single file
    const String &filename = input.getCmdOption("-f");
    if (!filename.IsEmpty()) {
        // Single DCU
        fList = fopen(AnsiString(filename).c_str(), "rt");
        if (!fList) {
            printf("[Error] Cannot open DCU list file: %s\n", argv[2]);
            fclose(fOut);
            return -1;
        }
        while (true) {
            if (!fgets(dcuFilename, 256, fList)) break;
            char *q = strrchr(dcuFilename, '\n');
            if (q) *q = 0;
            printf("Unit %s\n", dcuFilename);
            ScanOneDCU(String(dcuFilename));
            num++;
            printf("Done\n");
        }
        fclose(fList);
    }

    // Batch process directory
    const String &directory = input.getCmdOption("-d");
    if (!directory.IsEmpty()) {
        dirPath       = IncludeTrailingPathDelimiter(directory);
        searchPattern = dirPath + L"*.dcu";

        printf("Debug: Searching for DCU files in directory: %s\n", AnsiString(dirPath).c_str());

        if (FindFirst(searchPattern, iAttributes, sr) == 0) {
            try {
                do {
                    // Ignore directory attributes just in case
                    if ((sr.Attr & faDirectory) == 0) {
                        if (SameText(sr.Name.SubString(1,3), L"FMX")) {
                            continue;
                        }
                        printf("File %s\n", AnsiString(sr.Name).c_str());
                        String fullPath = dirPath + sr.Name;
                        if (!ScanOneDCU(fullPath)) {
                            printf("[Error] Unable to process file: %s\n", AnsiString(fullPath).c_str());
                        }
                        num++;
                        printf("Done\n");
                    }
                } while (FindNext(sr) == 0);
            } __finally {
                FindClose(sr);
            }
        }
    }

    /*if (argc == 3) {
        dirPath = IncludeTrailingPathDelimiter(argv[2]);
        searchPattern = dirPath + L"*.dcu";

        // fList = fopen(AnsiString(argv[2]).c_str(), "rt");
        // if (!fList) {
        //     printf("[Error] Cannot open DCU list file: %s\n", argv[2]);
        //     fclose(fOut);
        //     return -1;
        // }

        if (FindFirst(searchPattern, iAttributes, sr) == 0) {
            try {
                do {
                    // Ignore directory attributes just in case
                    if ((sr.Attr & faDirectory) == 0) {
                        printf("File %s\n", AnsiString(sr.Name).c_str());
                        String fullPath = dirPath + sr.Name;
                        if (!ScanOneDCU(fullPath)) {
                            printf("[Error] Unable to process file: %s\n", AnsiString(fullPath).c_str());
                        }
                        num++;
                        printf("Done\n");
                    }
                } while (FindNext(sr) == 0);
            } __finally {
                FindClose(sr);
            }
        }
    }*/

    // -sg: Is this correct?
    /*if (argc == 2) {
        // Several DCUs
        if (FindFirst("*.dcu", iAttributes, sr) == 0) {
            do {
                printf("Unit %s\n", sr.Name.c_str());
                ScanOneDCU(sr.Name);
                num++;
                printf("Done\n");
            } while (FindNext(sr) == 0);

            FindClose(sr);
        }
    } else {
        // Single DCU
        while (true) {
            if (!fgets(dcuFilename, 256, fList)) break;
            char *q = strrchr(dcuFilename, '\n');
            if (q) *q = 0;
            printf("Unit %s\n", dcuFilename);
            ScanOneDCU(String(dcuFilename));
            num++;
            printf("Done\n");
        }
        fclose(fList);
    }*/

    delete FUnitImp;
    delete FTypes;
    delete FAddrs;
    delete FTypeShowStack;

    printf("Processed %d files.\n", num);

    DWord CurrOffset = 0;
    FILE *fIn;
    long  fileLen;

    //---------------------------------------------------------------------
    // KB header
#ifdef NEW_VERSION
    const char *KBSignature = "IDR Knowledge Base File";
    DWord Version = 2.0;
#else
    const char *KBSignature = "IDD Knowledge Base File";
    DWord Version = 1.0;
#endif
    fwrite(KBSignature, 1, strlen(KBSignature) + 1, fOut);
    CurrOffset += strlen(KBSignature) + 1;
    fwrite(&IsMSIL, sizeof(IsMSIL), 1, fOut);
    CurrOffset += sizeof(IsMSIL);
    fwrite(&FVer, sizeof(FVer), 1, fOut);
    CurrOffset += sizeof(FVer);
    DWord CRC = 0xFFFFFFFF;
    fwrite(&CRC, sizeof(CRC), 1, fOut);
    CurrOffset += sizeof(CRC);
    char Description[256];
    fwrite(Description, 1, 256, fOut);
    CurrOffset += 256;
    fwrite(&Version, sizeof(Version), 1, fOut);
    CurrOffset += sizeof(Version);
    TDateTime CreateDT, LastModifyDT;
    fwrite(&CreateDT, sizeof(CreateDT), 1, fOut);
    CurrOffset += sizeof(CreateDT);
    fwrite(&LastModifyDT, sizeof(LastModifyDT), 1, fOut);
    CurrOffset += sizeof(LastModifyDT);

    //-----------------------------------------------------------------------
    // MODULES
    int ModuleCount = ModuleList->Count;
    int MaxModuleDataSize = 0;
    for (int n = 0; n < ModuleCount; n++) {
        ModuleInfo         = (PMODULEINFO) ModuleList->Items[n];
        ModuleInfo->ID     = n;
        ModuleInfo->Offset = CurrOffset;

        DWord DataSize = 0;
        // ModuleID
        fwrite(&ModuleInfo->ModuleID, sizeof(Word), 1, fOut);
        DataSize += sizeof(Word);
        // Name
        DataSize += WriteString(fOut, ModuleInfo->Name);
        // Filename
        DataSize += WriteString(fOut, ModuleInfo->Filename);
        // UsesNum
        Word UsesNum = ModuleInfo->UsesList->Count;
        fwrite(&UsesNum, sizeof(UsesNum), 1, fOut);
        DataSize += sizeof(UsesNum);
        // Uses
        for (int m = 0; m < UsesNum; m++) {
            bool found = false;
            for (int mm = 0; mm < ModuleCount; mm++) {
                PMODULEINFO modInfo = (PMODULEINFO) ModuleList->Items[mm];
                if (!AnsiString(modInfo->Name).AnsiCompareIC(ModuleInfo->UsesList->Strings[m])) {
                    fwrite(&modInfo->ModuleID, sizeof(Word), 1, fOut);
                    found = true;
                    break;
                }
            }
            if (!found) {
                Word mID = 0xFFFF;
                fwrite(&mID, sizeof(Word), 1, fOut);
                // Вспомогательная печать для определения замкнутого множества юнитов
                // Auxiliary printing for defining a closed set of units
                // if (!ModuleInfo->UsesList->Strings[m].Pos("."))
                //    printf("-%s\n", ModuleInfo->UsesList->Strings[m]);
            }
            DataSize += sizeof(Word);
        }
        for (int m = 0; m < UsesNum; m++) {
            DataSize += WriteString(fOut, String(ModuleInfo->UsesList->Strings[m]));
        }

        ModuleInfo->Size = DataSize;
        if (DataSize > MaxModuleDataSize) MaxModuleDataSize = DataSize;
        CurrOffset += DataSize;
    }
    fflush(fOut);
    fileLen = ftell(fOut);
    if (fileLen != CurrOffset) printf("Error: MODULES\n");

    //---------------------------------------------------------------------
    // CONSTANTS
    ConstList->Sort(CompareConstsByName);
    int    ConstCount       = 0;
    int    MaxConstDataSize = 0;
    Word   PrevModId        = 0xFFFF;
    String PrevName         = "";
    for (int n = 0; n < ConstList->Count; n++) {
        PCONSTINFO constInfo = (PCONSTINFO) ConstList->Items[n];
        constInfo->Skip      = false;
        if (constInfo->Name == PrevName && constInfo->ModuleID == PrevModId) {
            constInfo->Skip = true;
            continue;
        }

        constInfo->ID     = ConstCount;
        constInfo->Offset = CurrOffset;
        PrevModId         = constInfo->ModuleID;
        PrevName          = constInfo->Name;

        DWord DataSize = 0;
        // ModuleID
        fwrite(&constInfo->ModuleID, sizeof(constInfo->ModuleID), 1, fOut);
        DataSize += sizeof(constInfo->ModuleID);
        // Name
        DataSize += WriteString(fOut, constInfo->Name);
        // Type
        fwrite(&constInfo->Type, sizeof(constInfo->Type), 1, fOut);
        DataSize += sizeof(constInfo->Type);
        // TypeDef
        DataSize += WriteString(fOut, constInfo->TypeDef);
        // Value
        DataSize += WriteString(fOut, constInfo->Value);
        // Не будем дампить константы с внутренними именами
        // We will not dump constants with internal names
        if (constInfo->Name.Pos("_NV_") == 1) constInfo->RTTISz = 0;
        // DumpTotal
        DWord DumpTotal = 0;
        if (constInfo->RTTISz) {
            fIn = 0;
            for (int m = 0; m < ModuleList->Count; m++) {
                ModuleInfo = (PMODULEINFO) ModuleList->Items[m];
                if (ModuleInfo->ModuleID == constInfo->ModuleID) {
                    fIn = fopen(AnsiString(ModuleInfo->Filename).c_str(), "rb");
                    break;
                }
            }
            if (fIn) {
                // Dump
                DumpTotal += constInfo->RTTISz;
                // Relocs
                DumpTotal += constInfo->RTTISz;
                // Fixups
                DumpTotal += WriteFixups(0, constInfo->Fixups);
            }
        }
        DumpTotal += sizeof(constInfo->RTTISz); // DumpSz
        DWord FixupNum = constInfo->Fixups->Count;
        DumpTotal += sizeof(FixupNum); // FixupNum

        fwrite(&DumpTotal, sizeof(DumpTotal), 1, fOut);
        DataSize += sizeof(DumpTotal);
        // DumpSz
        fwrite(&constInfo->RTTISz, sizeof(constInfo->RTTISz), 1, fOut);
        DataSize += sizeof(constInfo->RTTISz);
        // FixupNum
        fwrite(&FixupNum, sizeof(FixupNum), 1, fOut);
        DataSize += sizeof(FixupNum);

        if (constInfo->RTTISz) {
            if (fIn) {
                // Dump
                DataSize += WriteDump(fIn, fOut, constInfo->RTTIOfs, constInfo->RTTISz);
                fclose(fIn);
                // Relocs
                DataSize += WriteRelocs(fOut, constInfo->Fixups, constInfo->RTTISz, constInfo->Name);
                // Fixups
                DataSize += WriteFixups(fOut, constInfo->Fixups);
            }
        }
        constInfo->Size = DataSize;
        if (DataSize > MaxConstDataSize) MaxConstDataSize = DataSize;
        CurrOffset += DataSize;
        ConstCount++;
    }
    fflush(fOut);
    fileLen = ftell(fOut);
    if (fileLen != CurrOffset) printf("Error: CONSTANTS\n");

    //-------------------------------------------------------------------------
    // TYPES
    int TypeCount       = TypeList->Count;
    int MaxTypeDataSize = 0;
    for (int n = 0; n < TypeCount; n++) {
        // TypeOffsets[n].DataOffset = CurrOffset;
        PTYPEINFO typeInfo = (PTYPEINFO) TypeList->Items[n];
        typeInfo->ID       = n;
        typeInfo->Offset   = CurrOffset;
        DWord DataSize     = 0;
#ifdef NEW_VERSION
        // Size
        fwrite(&typeInfo->Size, sizeof(typeInfo->Size), 1, fOut);
        DataSize += sizeof(typeInfo->Size);
#endif
        // ModuleID
        fwrite(&typeInfo->ModuleID, sizeof(typeInfo->ModuleID), 1, fOut);
        DataSize += sizeof(typeInfo->ModuleID);
        // Name
        DataSize += WriteString(fOut, typeInfo->Name);
        // Kind
        fwrite(&typeInfo->Kind, sizeof(typeInfo->Kind), 1, fOut);
        DataSize += sizeof(typeInfo->Kind);
        // VMCnt
        fwrite(&typeInfo->VMCnt, sizeof(typeInfo->VMCnt), 1, fOut);
        DataSize += sizeof(typeInfo->VMCnt);
        // Decl
        DataSize += WriteString(fOut, typeInfo->Decl);
        // Не будем дампить типы с внутренними именами
        // We will not dump types with internal names
        if (typeInfo->Name.Pos("_NT_") == 1) typeInfo->RTTISz = 0;
        // DumpTotal
        DWord DumpTotal = 0;
        if (typeInfo->RTTISz) {
            fIn = 0;
            for (int m = 0; m < ModuleList->Count; m++) {
                ModuleInfo = (PMODULEINFO) ModuleList->Items[m];
                if (ModuleInfo->ModuleID == typeInfo->ModuleID) {
                    fIn = fopen(AnsiString(ModuleInfo->Filename).c_str(), "rb");
                    break;
                }
            }
            if (fIn) {
                // Dump
                DumpTotal += typeInfo->RTTISz;
                // Relocs
                DumpTotal += typeInfo->RTTISz;
                // Fixups
                DumpTotal += WriteFixups(0, typeInfo->Fixups);
            }
        }
        DumpTotal += sizeof(typeInfo->RTTISz); // DumpSz
        DWord FixupNum = typeInfo->Fixups->Count;
        DumpTotal += sizeof(FixupNum); // FixupNum

        fwrite(&DumpTotal, sizeof(DumpTotal), 1, fOut);
        DataSize += sizeof(DumpTotal);
        // DumpSz
        fwrite(&typeInfo->RTTISz, sizeof(typeInfo->RTTISz), 1, fOut);
        DataSize += sizeof(typeInfo->RTTISz);
        // FixupNum
        fwrite(&FixupNum, sizeof(FixupNum), 1, fOut);
        DataSize += sizeof(FixupNum);

        if (typeInfo->RTTISz) {
            if (fIn) {
                // Dump
                DataSize += WriteDump(fIn, fOut, typeInfo->RTTIOfs, typeInfo->RTTISz);
                fclose(fIn);
                // Relocs
                DataSize += WriteRelocs(fOut, typeInfo->Fixups, typeInfo->RTTISz, typeInfo->Name);
                // Fixups
                DataSize += WriteFixups(fOut, typeInfo->Fixups);
            }
        }
        // FieldsTotal
        Word  FieldsNum   = typeInfo->Fields->Count;
        DWord FieldsTotal = 0;
        for (int m = 0; m < FieldsNum; m++) {
            PLOCALDECLINFO linfo = (PLOCALDECLINFO) typeInfo->Fields->Items[m];
            FieldsTotal += sizeof(linfo->Scope);
            FieldsTotal += sizeof(linfo->Ndx);
            FieldsTotal += sizeof(linfo->Case);
            FieldsTotal += WriteString(0, linfo->Name);
            FieldsTotal += WriteString(0, linfo->TypeDef);
        }
        FieldsTotal += sizeof(FieldsNum); // FieldsNum

        fwrite(&FieldsTotal, sizeof(FieldsTotal), 1, fOut);
        DataSize += sizeof(FieldsTotal);
        // FieldsNum
        fwrite(&FieldsNum, sizeof(FieldsNum), 1, fOut);
        DataSize += sizeof(FieldsNum);
        // Fields
        for (int m = 0; m < FieldsNum; m++) {
            PLOCALDECLINFO linfo = (PLOCALDECLINFO) typeInfo->Fields->Items[m];
            fwrite(&linfo->Scope, sizeof(linfo->Scope), 1, fOut);
            DataSize += sizeof(linfo->Scope);
            fwrite(&linfo->Ndx, sizeof(linfo->Ndx), 1, fOut);
            DataSize += sizeof(linfo->Ndx);
            fwrite(&linfo->Case, sizeof(linfo->Case), 1, fOut);
            DataSize += sizeof(linfo->Case);
            DataSize += WriteString(fOut, linfo->Name);
            DataSize += WriteString(fOut, linfo->TypeDef);
        }
        // PropsTotal
        Word  PropsNum   = typeInfo->Properties->Count;
        DWord PropsTotal = 0;
        for (int m = 0; m < PropsNum; m++) {
            if (typeInfo->Kind == drClassDef || typeInfo->Kind == drInterfaceDef) {
                PPROPERTYINFO pinfo = (PPROPERTYINFO) typeInfo->Properties->Items[m];
                PropsTotal += sizeof(pinfo->Scope);
                PropsTotal += sizeof(pinfo->Index);
                PropsTotal += sizeof(pinfo->DispId);
                PropsTotal += WriteString(0, pinfo->Name);
                PropsTotal += WriteString(0, pinfo->TypeDef);
                PropsTotal += WriteString(0, pinfo->ReadName);
                PropsTotal += WriteString(0, pinfo->WriteName);
                PropsTotal += WriteString(0, pinfo->StoredName);
            }
        }
        PropsTotal += sizeof(PropsNum); // PropsNum

        fwrite(&PropsTotal, sizeof(PropsTotal), 1, fOut);
        DataSize += sizeof(PropsTotal);
        // PropsNum
        fwrite(&PropsNum, sizeof(PropsNum), 1, fOut);
        DataSize += sizeof(PropsNum);
        // Props
        for (int m = 0; m < PropsNum; m++) {
            if (typeInfo->Kind == drClassDef || typeInfo->Kind == drInterfaceDef) {
                PPROPERTYINFO pinfo = (PPROPERTYINFO) typeInfo->Properties->Items[m];
                fwrite(&pinfo->Scope, sizeof(pinfo->Scope), 1, fOut);
                DataSize += sizeof(pinfo->Scope);
                fwrite(&pinfo->Index, sizeof(pinfo->Index), 1, fOut);
                DataSize += sizeof(pinfo->Index);
                fwrite(&pinfo->DispId, sizeof(pinfo->DispId), 1, fOut);
                DataSize += sizeof(pinfo->DispId);
                DataSize += WriteString(fOut, pinfo->Name);
                DataSize += WriteString(fOut, pinfo->TypeDef);
                DataSize += WriteString(fOut, pinfo->ReadName);
                DataSize += WriteString(fOut, pinfo->WriteName);
                DataSize += WriteString(fOut, pinfo->StoredName);
            }
        }
        // MethodsTotal
        Word  MethodsNum   = typeInfo->Methods->Count;
        DWord MethodsTotal = 0;
        for (int m = 0; m < MethodsNum; m++) {
            PMETHODDECLINFO minfo = (PMETHODDECLINFO) typeInfo->Methods->Items[m];
            MethodsTotal += sizeof(minfo->Scope);
            MethodsTotal += sizeof(minfo->MethodKind);
            MethodsTotal += WriteString(0, minfo->Prototype);
        }
        MethodsTotal += sizeof(MethodsNum); // MethodsNum

        fwrite(&MethodsTotal, sizeof(MethodsTotal), 1, fOut);
        DataSize += sizeof(MethodsTotal);
        // MethodsNum
        fwrite(&MethodsNum, sizeof(MethodsNum), 1, fOut);
        DataSize += sizeof(MethodsNum);
        // Methods
        for (int m = 0; m < MethodsNum; m++) {
            PMETHODDECLINFO minfo = (PMETHODDECLINFO) typeInfo->Methods->Items[m];
            fwrite(&minfo->Scope, sizeof(minfo->Scope), 1, fOut);
            DataSize += sizeof(minfo->Scope);
            fwrite(&minfo->MethodKind, sizeof(minfo->MethodKind), 1, fOut);
            DataSize += sizeof(minfo->MethodKind);
            DataSize += WriteString(fOut, minfo->Prototype);
        }
        typeInfo->Size = DataSize;
        if (DataSize > MaxTypeDataSize) MaxTypeDataSize = DataSize;
        CurrOffset += DataSize;
    }
    fflush(fOut);
    fileLen = ftell(fOut);
    if (fileLen != CurrOffset) printf("Error: TYPES\n");

    //--------------------------------------------------------------------------
    // VARS
    int VarCount       = VarList->Count;
    int MaxVarDataSize = 0;
    for (int n = 0; n < VarCount; n++) {
        PVARINFO vInfo = (PVARINFO) VarList->Items[n];
        vInfo->ID      = n;
        vInfo->Offset  = CurrOffset;

        DWord DataSize = 0;
        // ModuleID
        fwrite(&vInfo->ModuleID, sizeof(vInfo->ModuleID), 1, fOut);
        DataSize += sizeof(vInfo->ModuleID);
        // Name
        DataSize += WriteString(fOut, vInfo->Name);
        // Type
        fwrite(&vInfo->Type, sizeof(vInfo->Type), 1, fOut);
        DataSize += sizeof(vInfo->Type);
        // TypeDef
        DataSize += WriteString(fOut, vInfo->TypeDef);
        // AbsName
        DataSize += WriteString(fOut, vInfo->AbsName);

        vInfo->Size = DataSize;
        if (DataSize > MaxVarDataSize) MaxVarDataSize = DataSize;
        CurrOffset += DataSize;
    }
    fflush(fOut);
    fileLen = ftell(fOut);
    if (fileLen != CurrOffset) printf("Error: VARS\n");

    //--------------------------------------------------------------
    // RESOURCE STRINGS
    int  ResStrCount       = ResStrList->Count;
    int  MaxResStrDataSize = 0;
    Byte ResStrBuf[1024];
    for (int n = 0; n < ResStrCount; n++) {
        PRESSTRINFO rsInfo = (PRESSTRINFO) ResStrList->Items[n];
        rsInfo->ID         = n;
        rsInfo->Offset     = CurrOffset;

        DWord DataSize = 0;
        // ModuleID
        fwrite(&rsInfo->ModuleID, sizeof(rsInfo->ModuleID), 1, fOut);
        DataSize += sizeof(rsInfo->ModuleID);
        // Name
        DataSize += WriteString(fOut, rsInfo->Name);
        // TypeDef
        DataSize += WriteString(fOut, rsInfo->TypeDef);
        // Context
        if (rsInfo->DumpSz) {
            fIn = 0;
            for (int m = 0; m < ModuleList->Count; m++) {
                ModuleInfo = (PMODULEINFO) ModuleList->Items[m];
                if (ModuleInfo->ModuleID == rsInfo->ModuleID) {
                    fIn = fopen(AnsiString(ModuleInfo->Filename).c_str(), "rb");
                    break;
                }
            }
            if (fIn) {
                // Context
                fseek(fIn, rsInfo->DumpOfs, SEEK_SET);
                fread(ResStrBuf, 1, rsInfo->DumpSz, fIn);
                fclose(fIn);
                Word ResStrLen  = *reinterpret_cast<Word *>(ResStrBuf + 4);
                rsInfo->Context = String(reinterpret_cast<char *>(ResStrBuf + 8), ResStrLen);
                DataSize += WriteString(fOut, rsInfo->Context);
            }
        }
        rsInfo->Size = DataSize;
        if (DataSize > MaxResStrDataSize) MaxResStrDataSize = DataSize;
        CurrOffset += DataSize;
    }
    fflush(fOut);
    fileLen = ftell(fOut);
    if (fileLen != CurrOffset) printf("Error: RESOURCE STRINGS\n");

    //--------------------------------------------------------------------
    // PROCEDURES
    int ProcCount       = ProcList->Count;
    int MaxProcDataSize = 0;
    for (int n = 0; n < ProcCount; n++) {
        DWord         DataSize = 0;
        PPROCDECLINFO pInfo    = (PPROCDECLINFO) ProcList->Items[n];
        pInfo->ID              = n;
        pInfo->Offset          = CurrOffset;
        // ModuleID
        fwrite(&pInfo->ModuleID, sizeof(pInfo->ModuleID), 1, fOut);
        DataSize += sizeof(pInfo->ModuleID);
        // Name
        DataSize += WriteString(fOut, pInfo->Name);
        // Embedded
        fwrite(&pInfo->Embedded, sizeof(pInfo->Embedded), 1, fOut);
        DataSize += sizeof(pInfo->Embedded);
        // DumpType
        fwrite(&pInfo->DumpType, sizeof(pInfo->DumpType), 1, fOut);
        DataSize += sizeof(pInfo->DumpType);
        // MethodKind
        fwrite(&pInfo->MethodKind, sizeof(pInfo->MethodKind), 1, fOut);
        DataSize += sizeof(pInfo->MethodKind);
        // CallKind
        fwrite(&pInfo->CallKind, sizeof(pInfo->CallKind), 1, fOut);
        DataSize += sizeof(pInfo->CallKind);
        // VProc
        fwrite(&pInfo->VProc, sizeof(pInfo->VProc), 1, fOut);
        DataSize += sizeof(pInfo->VProc);
        // TypeDef
        DataSize += WriteString(fOut, pInfo->TypeDef);
        // DumpTotal
        DWord DumpTotal = 0;
        if (pInfo->DumpSz) {
            fIn = 0;
            for (int m = 0; m < ModuleList->Count; m++) {
                ModuleInfo = (PMODULEINFO) ModuleList->Items[m];
                if (ModuleInfo->ModuleID == pInfo->ModuleID) {
                    fIn = fopen(AnsiString(ModuleInfo->Filename).c_str(), "rb");
                    break;
                }
            }
            if (fIn) {
                // Dump
                DumpTotal += pInfo->DumpSz;
                // Relocs
                DumpTotal += pInfo->DumpSz;
                // Fixups
                DumpTotal += WriteFixups(0, pInfo->Fixups);
            }
        }
        DumpTotal += sizeof(pInfo->DumpSz); // DumpSz
        DWord FixupNum = pInfo->Fixups->Count;
        DumpTotal += sizeof(FixupNum); // FixupNum

        fwrite(&DumpTotal, sizeof(DumpTotal), 1, fOut);
        DataSize += sizeof(DumpTotal);
        // DumpSz
        fwrite(&pInfo->DumpSz, sizeof(pInfo->DumpSz), 1, fOut);
        DataSize += sizeof(pInfo->DumpSz);
        // FixupNum
        fwrite(&FixupNum, sizeof(FixupNum), 1, fOut);
        DataSize += sizeof(FixupNum);

        if (pInfo->DumpSz) {
            if (fIn) {
                // Dump
                DataSize += WriteDump(fIn, fOut, pInfo->DumpOfs, pInfo->DumpSz);
                fclose(fIn);
                // Relocs
                DataSize += WriteRelocs(fOut, pInfo->Fixups, pInfo->DumpSz, pInfo->Name);
                // Fixups
                DataSize += WriteFixups(fOut, pInfo->Fixups);
            }
        }
        // ArgsTotal
        Word  ArgsNum   = pInfo->Args->Count;
        DWord ArgsTotal = 0;
        for (int m = 0; m < ArgsNum; m++) {
            PLOCALDECLINFO lInfo = (PLOCALDECLINFO) pInfo->Args->Items[m];
            ArgsTotal += sizeof(lInfo->Tag);
            ArgsTotal += sizeof(lInfo->LocFlags);
            ArgsTotal += sizeof(lInfo->Ndx);
            ArgsTotal += WriteString(0, lInfo->Name);
            ArgsTotal += WriteString(0, lInfo->TypeDef);
        }
        ArgsTotal += sizeof(ArgsNum); // ArgsNum

        fwrite(&ArgsTotal, sizeof(ArgsTotal), 1, fOut);
        DataSize += sizeof(ArgsTotal);
        // ArgsNum
        fwrite(&ArgsNum, sizeof(ArgsNum), 1, fOut);
        DataSize += sizeof(ArgsNum);
        // Args
        for (int m = 0; m < ArgsNum; m++) {
            PLOCALDECLINFO lInfo = (PLOCALDECLINFO) pInfo->Args->Items[m];

            fwrite(&lInfo->Tag, sizeof(lInfo->Tag), 1, fOut);
            DataSize += sizeof(lInfo->Tag);
            fwrite(&lInfo->LocFlags, sizeof(lInfo->LocFlags), 1, fOut);
            DataSize += sizeof(lInfo->LocFlags);
            fwrite(&lInfo->Ndx, sizeof(lInfo->Ndx), 1, fOut);
            DataSize += sizeof(lInfo->Ndx);
            DataSize += WriteString(fOut, lInfo->Name);
            DataSize += WriteString(fOut, lInfo->TypeDef);
        }
        // LocalsTotal
        Word  LocalsNum   = pInfo->Locals->Count;
        DWord LocalsTotal = 0;
        for (int m = 0; m < LocalsNum; m++) {
            PLOCALDECLINFO lInfo = (PLOCALDECLINFO) pInfo->Locals->Items[m];
            LocalsTotal += sizeof(lInfo->Tag);
            LocalsTotal += sizeof(lInfo->LocFlags);
            LocalsTotal += sizeof(lInfo->Ndx);
            LocalsTotal += WriteString(0, lInfo->Name);
            LocalsTotal += WriteString(0, lInfo->TypeDef);
            LocalsTotal += WriteString(0, lInfo->AbsName);
        }
        LocalsTotal += sizeof(LocalsNum); // LocalsNum

        fwrite(&LocalsTotal, sizeof(LocalsTotal), 1, fOut);
        DataSize += sizeof(LocalsTotal);
        // LocalsNum
        fwrite(&LocalsNum, sizeof(LocalsNum), 1, fOut);
        DataSize += sizeof(LocalsNum);
        // Locals
        for (int m = 0; m < LocalsNum; m++) {
            PLOCALDECLINFO lInfo = (PLOCALDECLINFO) pInfo->Locals->Items[m];

            fwrite(&lInfo->Tag, sizeof(lInfo->Tag), 1, fOut);
            DataSize += sizeof(lInfo->Tag);
            fwrite(&lInfo->LocFlags, sizeof(lInfo->LocFlags), 1, fOut);
            DataSize += sizeof(lInfo->LocFlags);
            fwrite(&lInfo->Ndx, sizeof(lInfo->Ndx), 1, fOut);
            DataSize += sizeof(lInfo->Ndx);
            DataSize += WriteString(fOut, lInfo->Name);
            DataSize += WriteString(fOut, lInfo->TypeDef);
            DataSize += WriteString(fOut, lInfo->AbsName);
        }
        pInfo->Size = DataSize;
        if (DataSize > MaxProcDataSize) MaxProcDataSize = DataSize;
        CurrOffset += DataSize;
        fflush(fOut);
        fileLen = ftell(fOut);
        if (fileLen != CurrOffset) printf("Error: PROCEDURES\n");
    }
    fflush(fOut);
    fileLen = ftell(fOut);
    if (fileLen != CurrOffset) printf("Error: PROCEDURES\n");

    //----------------------------------------------------------------
    // Module Section
    fwrite(&ModuleCount, sizeof(ModuleCount), 1, fOut);
    fwrite(&MaxModuleDataSize, sizeof(MaxModuleDataSize), 1, fOut);

    POFFSETSINFO ModuleOffsets = new OFFSETSINFO[ModuleCount];
    for (int n = 0; n < ModuleCount; n++) {
        ModuleInfo              = (PMODULEINFO) ModuleList->Items[n];
        ModuleOffsets[n].Offset = ModuleInfo->Offset;
        ModuleOffsets[n].Size   = ModuleInfo->Size;
    }
    ModuleList->Sort(CompareModulesByID);
    for (int n = 0; n < ModuleCount; n++) {
        ModuleInfo             = (PMODULEINFO) ModuleList->Items[n];
        ModuleOffsets[n].ModId = ModuleInfo->ID;
    }
    ModuleList->Sort(CompareModulesByName);
    for (int n = 0; n < ModuleCount; n++) {
        ModuleInfo             = (PMODULEINFO) ModuleList->Items[n];
        ModuleOffsets[n].NamId = ModuleInfo->ID;
    }
    fwrite(ModuleOffsets, sizeof(OFFSETSINFO), ModuleCount, fOut);
    delete[] ModuleOffsets;

    //-----------------------------------------------------------------
    // Const Section (KB_CONST_SECTION = 1)
    fwrite(&ConstCount, sizeof(ConstCount), 1, fOut);
    fwrite(&MaxConstDataSize, sizeof(MaxConstDataSize), 1, fOut);

    POFFSETSINFO ConstOffsets = new OFFSETSINFO[ConstCount];

    int cn = 0;

    for (int n = 0; n < ConstList->Count; n++) {
        PCONSTINFO constInfo = (PCONSTINFO) ConstList->Items[n];
        if (constInfo->Skip) continue;

        ConstOffsets[cn].Offset = constInfo->Offset;
        ConstOffsets[cn].Size   = constInfo->Size;
        cn++;
    }
    ConstList->Sort(CompareConstsByID);
    cn = 0;
    for (int n = 0; n < ConstList->Count; n++) {
        PCONSTINFO constInfo = (PCONSTINFO) ConstList->Items[n];
        if (constInfo->Skip) continue;

        ConstOffsets[cn].ModId = constInfo->ID;
        cn++;
    }
    ConstList->Sort(CompareConstsByName);
    cn = 0;
    for (int n = 0; n < ConstList->Count; n++) {
        PCONSTINFO constInfo = (PCONSTINFO) ConstList->Items[n];
        if (constInfo->Skip) continue;

        ConstOffsets[cn].NamId = constInfo->ID;
        cn++;
    }
    fwrite(ConstOffsets, sizeof(OFFSETSINFO), ConstCount, fOut);
    delete[] ConstOffsets;

    //------------------------------------------------------------------
    // Type Section (KB_TYPE_SECTION = 2)
    fwrite(&TypeCount, sizeof(TypeCount), 1, fOut);
    fwrite(&MaxTypeDataSize, sizeof(MaxTypeDataSize), 1, fOut);

    POFFSETSINFO TypeOffsets = new OFFSETSINFO[TypeCount];
    for (int n = 0; n < TypeCount; n++) {
        PTYPEINFO typeInfo    = (PTYPEINFO) TypeList->Items[n];
        TypeOffsets[n].Offset = typeInfo->Offset;
        TypeOffsets[n].Size   = typeInfo->Size;
    }
    TypeList->Sort(CompareTypesByID);
    for (int n = 0; n < TypeCount; n++) {
        PTYPEINFO typeInfo   = (PTYPEINFO) TypeList->Items[n];
        TypeOffsets[n].ModId = typeInfo->ID;
    }
    TypeList->Sort(CompareTypesByName);
    for (int n = 0; n < TypeCount; n++) {
        PTYPEINFO typeInfo   = (PTYPEINFO) TypeList->Items[n];
        TypeOffsets[n].NamId = typeInfo->ID;
    }
    fwrite(TypeOffsets, sizeof(OFFSETSINFO), TypeCount, fOut);
    delete[] TypeOffsets;

    //-------------------------------------------------------------------
    // Var Section (KB_VAR_SECTION = 4)
    fwrite(&VarCount, sizeof(VarCount), 1, fOut);
    fwrite(&MaxVarDataSize, sizeof(MaxVarDataSize), 1, fOut);

    POFFSETSINFO VarOffsets = new OFFSETSINFO[VarCount];
    for (int n = 0; n < VarCount; n++) {
        PVARINFO vInfo       = (PVARINFO) VarList->Items[n];
        VarOffsets[n].Offset = vInfo->Offset;
        VarOffsets[n].Size   = vInfo->Size;
    }
    VarList->Sort(CompareVarsByID);
    for (int n = 0; n < VarCount; n++) {
        PVARINFO vInfo      = (PVARINFO) VarList->Items[n];
        VarOffsets[n].ModId = vInfo->ID;
    }
    VarList->Sort(CompareVarsByName);
    for (int n = 0; n < VarCount; n++) {
        PVARINFO vInfo      = (PVARINFO) VarList->Items[n];
        VarOffsets[n].NamId = vInfo->ID;
    }
    fwrite(VarOffsets, sizeof(OFFSETSINFO), VarCount, fOut);
    delete[] VarOffsets;

    //----------------------------------------------------------------
    // ResStr Section (KB_RESSTR_SECTION = 8)
    fwrite(&ResStrCount, sizeof(ResStrCount), 1, fOut);
    fwrite(&MaxResStrDataSize, sizeof(MaxResStrDataSize), 1, fOut);

    POFFSETSINFO ResStrOffsets = new OFFSETSINFO[ResStrCount];
    for (int n = 0; n < ResStrCount; n++) {
        PRESSTRINFO rsInfo      = (PRESSTRINFO) ResStrList->Items[n];
        ResStrOffsets[n].Offset = rsInfo->Offset;
        ResStrOffsets[n].Size   = rsInfo->Size;
    }
    ResStrList->Sort(CompareResStrsByID);
    for (int n = 0; n < ResStrCount; n++) {
        PRESSTRINFO rsInfo     = (PRESSTRINFO) ResStrList->Items[n];
        ResStrOffsets[n].ModId = rsInfo->ID;
    }
    ResStrList->Sort(CompareResStrsByName);
    for (int n = 0; n < ResStrCount; n++) {
        PRESSTRINFO rsInfo     = (PRESSTRINFO) ResStrList->Items[n];
        ResStrOffsets[n].NamId = rsInfo->ID;
    }
    fwrite(ResStrOffsets, sizeof(OFFSETSINFO), ResStrCount, fOut);
    delete[] ResStrOffsets;

    //------------------------------------------------------------------
    // Proc Section (KB_PROC_SECTION = 16)
    fwrite(&ProcCount, sizeof(ProcCount), 1, fOut);
    fwrite(&MaxProcDataSize, sizeof(MaxProcDataSize), 1, fOut);

    POFFSETSINFO ProcOffsets = new OFFSETSINFO[ProcCount];
    for (int n = 0; n < ProcCount; n++) {
        PPROCDECLINFO pInfo   = (PPROCDECLINFO) ProcList->Items[n];
        ProcOffsets[n].Offset = pInfo->Offset;
        ProcOffsets[n].Size   = pInfo->Size;
    }
    ProcList->Sort(CompareProcsByID);
    for (int n = 0; n < ProcCount; n++) {
        PPROCDECLINFO pInfo  = (PPROCDECLINFO) ProcList->Items[n];
        ProcOffsets[n].ModId = pInfo->ID;
    }
    ProcList->Sort(CompareProcsByName);
    for (int n = 0; n < ProcCount; n++) {
        PPROCDECLINFO pInfo  = (PPROCDECLINFO) ProcList->Items[n];
        ProcOffsets[n].NamId = pInfo->ID;
    }
    fwrite(ProcOffsets, sizeof(OFFSETSINFO), ProcCount, fOut);
    delete[] ProcOffsets;

    //----------------------------------------------------------------
    // Sections begin
    fwrite(&CurrOffset, sizeof(CurrOffset), 1, fOut);
    fclose(fOut);
    if (fLog) fclose(fLog);

    ModuleList->Clear();

    delete ModuleList;
    delete ConstList;
    delete TypeList;
    delete VarList;
    delete ResStrList;
    delete ProcList;
    
    printf("Debug: Finished writing KB file: %s\n", AnsiString(output).c_str());
    
    return 0;
}
//------------------------------------------------------------------------------
