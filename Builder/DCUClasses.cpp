#include <vcl.h>
#include <System.hpp>
#include <System.AnsiStrings.hpp>
#include <typeinfo.h>

#include "DCUClasses.h"
#include "Main.h"
//------------------------------------------------------------------------------
extern DWord        *pDumpOffset;
extern DWord        *pDumpSize;
extern TList        *FixupsList;
extern Byte         ActiveScope;
extern TList        *FieldsList;
extern TList        *PropertiesList;
extern TList        *MethodsList;
extern TList        *ProcsList;
extern TList        *ArgsList;
extern TList        *LocalsList;
extern FILE         *fOut;
extern bool         ThreadVar;
extern Word         ModuleID;
extern TList        *ConstList;
extern TList        *TypeList;
extern TList        *VarList;
extern TList        *ResStrList;
extern TList        *ProcList;
extern int          CaseN;
extern bool         GenVarCAsVars;
extern int          FVer;
extern bool         IsMSIL;
extern int          NDXHi;
extern Byte         *DefStart;
extern TIncPtr      CurPos;
extern Byte         Tag;
extern int          FTypeDefCnt;
extern Byte         *FDataBlPtr;
extern int          FFixupCnt;
extern TFixupRec    *FFixupTbl;
extern Byte         fxStart; 
extern TNameRec     NoName;
extern FILE         *fLog;
extern int          FPlatform;
extern bool         FromPackage;
extern int          FPtrSize;
extern TFxSizeTbl   fxSize;
extern bool         fx8Byte;
extern System::Set<std::uint8_t, 0, fxMax> fxValid;

//------------------------------------------------------------------------------
TDCURec::TDCURec() : TObject() { Next = 0; }
//------------------------------------------------------------------------------
PName __fastcall TDCURec::GetName() { return nullptr; }
//------------------------------------------------------------------------------
DWord __fastcall TDCURec::SetMem(DWord MOfs, DWord MSz) { return 0; }
//------------------------------------------------------------------------------
bool __fastcall TDCURec::NameIsUnique() { return false; }
//------------------------------------------------------------------------------
void __fastcall TDCURec::Visit(TDCURecVisitor *Visitor) {}
//------------------------------------------------------------------------------
void __fastcall TDCURec::ShowName(String &OutS) { OutS = ""; }
//------------------------------------------------------------------------------
void __fastcall TDCURec::Show(String &OutS) { OutS = ""; }
//------------------------------------------------------------------------------
// for verD_XE - fix orphaned local types problem
void __fastcall TDCURec::EnumUsedTypes(TTypeUseAction Action, DWord *IP) {
    // no type used
}
//------------------------------------------------------------------------------
TDCURecTag __fastcall TDCURec::GetTag() { return 0; }
//------------------------------------------------------------------------------
bool __fastcall TDCURec::IsVisible(Byte LK) { return false; }
//------------------------------------------------------------------------------
Byte __fastcall TDCURec::GetSecKind() {
    // todo: return TDeclSecKind::None;
    return skNone;
}
//------------------------------------------------------------------------------
void __fastcall TDCURec::ShowDef(bool All, String &OutS) { Show(OutS); }
//------------------------------------------------------------------------------
void TDCURec::ListAppend(TDCURec *List) {
    Next = List;
    List = this;
}
//------------------------------------------------------------------------------
void TDCURec::SetSegKind(TSegKind V) {}
//------------------------------------------------------------------------------
TBaseDef::TBaseDef(PName AName, PDef ADef, int AUnit) : TDCURec() {
    FName = AName;
    Def   = ADef;
    hUnit = AUnit;
}
//------------------------------------------------------------------------------
void TBaseDef::Visit(TDCURecVisitor *Visitor) { Visitor->visitBaseDef(this); }
//------------------------------------------------------------------------------
void __fastcall TBaseDef::ShowName(String &OutS) {
    PName NP = FName;
    if (!NP || NP->IsEmpty()) NP = &NoName;
    if (hUnit < 0) {
        if (!NP->IsEmpty()) {
            // DCU32: PutDCURecStr(Self,hDecl,false);
            OutS = GetDCURecStr(this, hDecl);
            OutLog2("%s", OutS.c_str());
        }
    } else if (NameIsUnique()) {
        // DCU32: PutAddrDefStr(NP^.GetStr,hDecl)
        OutS = NP->GetStr();
        OutLog2("%s", OutS.c_str());
    } else {
        PUnitImpRec U = GetUnitImpRec(hUnit);
        OutS = U->Name->GetStr() + "." + NP->GetStr();
        OutLog2("%s", OutS.c_str());
    }
}
//------------------------------------------------------------------------------
void __fastcall TBaseDef::Show(String &OutS) {
    PName NP = FName;
    if (!NP || NP->IsEmpty()) NP = &NoName;
    OutS = NP->GetStr();
    OutLog2("%s", OutS.c_str());
}
//------------------------------------------------------------------------------
void __fastcall TBaseDef::ShowNamed(PName N, String &OutS) {
    // was: if ((N && N == FName || !FName || FName->Len) && RegTypeShow(this)) {
    if (((N && N == FName) ||                     // Showing the definition of N
         (!FName) || !FName->IsEmpty() ||               // No name
         (FName->IsAuxName() && dynamic_cast<TTypeDef*>(this)))
         && RegTypeShow(this)) { // if RegTypeShow fails the type name will be shown instead of its definition
        try {
            Show(OutS);
        } catch (...) {
            UnRegTypeShow(this);
        }
    } else {
        ShowName(OutS);
    }
}
//------------------------------------------------------------------------------
PName __fastcall TBaseDef::GetName() {
    if (!FName) {
        return &NoName;
    }
    return FName;
}
//------------------------------------------------------------------------------
DWord __fastcall TBaseDef::SetMem(DWord MOfs, DWord MSz) { return 0; }
//------------------------------------------------------------------------------
TImpDef::TImpDef(TImpKind AIK, PName AName, int AnInf, PDef ADef, int AUnit) : TBaseDef(AName, ADef, AUnit) {
    Inf = AnInf;
    ik  = AIK;
}
//------------------------------------------------------------------------------
void TImpDef::Visit(TDCURecVisitor *Visitor) { Visitor->visitImpDef(this); }
//------------------------------------------------------------------------------
void __fastcall TImpDef::Show(String &OutS) {
    String S;
    OutS = String(ik) + ":";
    OutLog2("%s:", OutS.c_str());
    TBaseDef::Show(S);
    OutS += S;
}
//------------------------------------------------------------------------------
bool __fastcall TImpDef::NameIsUnique() { return FNameIsUnique; }
//------------------------------------------------------------------------------
TUnitImpDef::TUnitImpDef(TImpKind AIK, PName AName, int AnInf, PDef ADef, int AUnit) : TImpDef(AIK, AName, AnInf, ADef, AUnit) {}
//------------------------------------------------------------------------------
void TUnitImpDef::Show(String &OutS) {
    TImpDef::Show(OutS);
    if (sPackage != "") {
        OutLog1(sPackage.c_str());
    }
}
//------------------------------------------------------------------------------
void TUnitImpDef::Visit(TDCURecVisitor *Visitor) { Visitor->visitUnitImpDef(this); }
//------------------------------------------------------------------------------
TDLLImpRec::TDLLImpRec(PName AName, int ANdx, PDef ADef, int AUnit) : TBaseDef(AName, ADef, AUnit) { Ndx = ANdx; }
//------------------------------------------------------------------------------
void TDLLImpRec::Visit(TDCURecVisitor *Visitor) { Visitor->visitDLLImpRec(this); }
//------------------------------------------------------------------------------
void __fastcall TDLLImpRec::Show(String &OutS) {
    OutS          = "";
    bool   NoName = (!FName || FName->IsEmpty());
    AnsiString Name;
    if (!NoName) {
        Name = FName->GetStr();
        OutS += "name '" + Name + "'";
        OutLog2("name '%s'", Name.c_str());
    }
    if (NoName || Ndx) {
        OutS += "index " + IntToHex(Ndx, 0);
        OutLog2("index %lX", Ndx);
    }
}
//------------------------------------------------------------------------------
TImpTypeDefRec::TImpTypeDefRec(PName AName, int AnInf, DWord ARTTISz, PDef ADef, int AUnit) :
    TImpDef('T', AName, AnInf, ADef, AUnit) {
    RTTISz   = ARTTISz;
    RTTIOfs  = -1;
    hImpUnit = hUnit;
    hUnit    = -1;
    ImpName  = FName;
    FName    = nullptr; // Will be named later in the corresponding TTypeDecl
}
//------------------------------------------------------------------------------
void TImpTypeDefRec::Visit(TDCURecVisitor *Visitor) { Visitor->visitImpTypeDefRec(this); }
//------------------------------------------------------------------------------
void __fastcall TImpTypeDefRec::Show(String &OutS) {
    AnsiString Name;
    OutS = "type ";
    OutLog1("type ");
    if (hImpUnit >= 0) {
        PUnitImpRec U = GetUnitImpRec(hImpUnit);
        Name          = PName2String(U->Name);
        OutS += Name + ".";
        OutLog2("%s.", Name.c_str());
    }
    Name = PName2String(ImpName);
    OutS += Name;
    OutLog2("%s", Name.c_str());
    if (RTTISz > 0) {
        OutLog1("RTTI: ");
        ShowDataBl(0, RTTIOfs, RTTISz);
    }
}
//------------------------------------------------------------------------------
DWord __fastcall TImpTypeDefRec::SetMem(DWord MOfs, DWord MSz) {
    RTTIOfs = MOfs;
    return 0;
}
//------------------------------------------------------------------------------
void TConstValInfoBase::Show0(TDefNDX hDT, bool IsNamed) {
    Byte *DP;
    DWord DS;
    TInt64Rec V;
    String S;

    if (!ValPtr) {
        V.Hi = ValSz;
        V.Lo = Val;
        DP   = reinterpret_cast<Byte *>(&V);
        DS   = 8;
    } else {
        DP = ValPtr;
        DS = ValSz;
    }

    bool MemVal = ValPtr ? true : false;

    if ((ShowGlobalTypeValue(hDT, DP, DS, MemVal, Kind, IsNamed, S) < 0) && !MemVal) {
        ShowTypeName(hDT);
        NDXHi = V.Hi;
        OutLog2("(%s)", NDXToStr(V.Lo).c_str());
    }
}
//------------------------------------------------------------------------------
// todo: review:
void TConstValInfo::Read() {
    bool NeedVal = true;

    if (FVer > verD4) {
        Kind = ReadUIndex();

        if ((Kind < 0 || Kind > 5 || Kind == 5) && !(FVer >= verD2009 && FVer < verK1)) {
            printf("[Error] TConstValInfo::Read: Unknown const kind: #%d\n", Kind); // DCUErrorFmt
        }
        if (FVer >= verDXE2 && FVer < verK1) {
            NeedVal = Kind != 4;
        }
    }

    ValSz = ReadUIndex();
    if (ValSz == 0) {
        ValPtr = nullptr;
        if (NeedVal) {
            Val = ReadIndex();
        }
        ValSz = NDXHi;
    } else {
        ValPtr = ReadMem(ValSz);
        Val    = 0;
    }
}
//------------------------------------------------------------------------------
void TConstValInfo::Show(bool IsNamed) {
    Show0(hDT, IsNamed);
}
//------------------------------------------------------------------------------
TDeclModifier::TDeclModifier() {}
//------------------------------------------------------------------------------
TDeclModifier::~TDeclModifier() { delete Next; }
//------------------------------------------------------------------------------
void __fastcall TDeclModifier::Show(String &OutS) {}
//------------------------------------------------------------------------------
bool __fastcall TDeclModifier::ShowBefore() { return false; }
//------------------------------------------------------------------------------
// Name Declaration Modifiers - store some important information from
// drConstAddInfo records and other records like this if any
TDeclModifier * __fastcall GetDeclModifierOfClass(TDeclModifier *L, TDeclModifierClass *cl) {
    TDeclModifier *result = L;
    while (result && !(result->InheritsFrom(__classid(TDeclModifierClass)))) {
        result = result->Next;
    }
    return result;
}
//------------------------------------------------------------------------------
TDeclModifier *__fastcall TDeclModifier::GetNextOfClass(TDeclModifierClass *Cl) {
    return GetDeclModifierOfClass(Next, Cl);
}
//------------------------------------------------------------------------------
TStrDeclModifier::TStrDeclModifier(TMemStrRef *AMsg) : TDeclModifier() {
    FMsg = AMsg;
}
//------------------------------------------------------------------------------
void TDeprecatedDeclModifier::Show(String &OutS) {
    OutS = " deprecated";
    if (FMsg && FMsg->Len() > 0) OutS += " ";
    OutS += System::Ansistrings::AnsiQuotedStr(FMsg->S(), '\'');
    OutLog2("%s", OutS.c_str());

    // PutSpace;
    // PutKW('deprecated');
    // if FMsg.Len>0 then begin
    //   PutSpace;
    // PutStrConstQ(Msg);
    // end ;
}
//------------------------------------------------------------------------------
TXMLDocDeclModifier::TXMLDocDeclModifier(TMemStrRef *AMsg) : TStrDeclModifier(AMsg) {}
//------------------------------------------------------------------------------
void TXMLDocDeclModifier::Show(String &OutS) {
    OutS = "///";
    OutS += String(FMsg->S());
    OutLog1("///");
    OutLog2("%s\n", AnsiString(FMsg->S()).c_str());
}
//------------------------------------------------------------------------------
bool TXMLDocDeclModifier::ShowBefore() { return true; }
//------------------------------------------------------------------------------
// TAttributeDeclModifier.Read:
TAttributeDeclModifier::TAttributeDeclModifier() : TDeclModifier() {
    hAttrCtor = ReadUIndex();
    RefAddrDef(hAttrCtor);
    hMember = ReadUIndex();
    hAttrDT = ReadUIndex();
    ArgCnt  = ReadUIndex();
    Args = static_cast<PAttributeDeclArgs>(AllocMem(ArgCnt * sizeof(TAttributeDeclArg)));

    for (int j = 0; j < ArgCnt; j++) {
        (*Args)[j].Kind = ReadUIndex();

        switch ((*Args)[j].Kind) {
            case 0: // const
                (*Args)[j].C.hDT = ReadUIndex();
                (*Args)[j].C.Read();
                break;
            case 1: // TypeInfo(DT)
                (*Args)[j].A.hDT = ReadUIndex(); // DT index in the type table
                (*Args)[j].A.hDTAddr = ReadUIndex(); // DT index in the addr table
                RefAddrDef((*Args)[j].A.hDTAddr);
                break;
            default:
                printf("[Error] Unexpected argument kind: %d in attribute argument table", (*Args)[j].Kind); // DCUErrorFmt
                break;
        }
    }
}
//------------------------------------------------------------------------------
TAttributeDeclModifier::~TAttributeDeclModifier() {
    if (Args) delete[] Args;
}
//------------------------------------------------------------------------------
void TAttributeDeclModifier::Show(String &OutS) {
    // todo: review
    static const char sAttr[] = "Attribute";
    static const int  lAttr   = sizeof(sAttr) - 1;

    PName NP = GetTypeName(hAttrDT);
    if (!NP)
        return;

    AnsiString S = NP->GetStr();
    if (S.IsEmpty())
        return;

    int L = S.Length();

    if (L > lAttr) {
        const char *CP = S.c_str() + L - lAttr;

        if (System::Ansistrings::StrLIComp(CP, sAttr, lAttr) == 0) {
            S.SetLength(L - lAttr);
        }
    }

    OutS += "[";
    OutLog1("[");

    if (hMember != 0) {
        if (hMember == 0x0D) {
            OutS += "Result";
            OutLog1("Result");
        } else {
            String Member = Sysutils::Format("?#%x", ARRAYOFCONST((hMember)));
            OutS += Member;
            OutLog2("%s", Member.c_str());
        }

        OutS += ":";
        OutLog1(":");
    }

    OutS += S;
    OutLog2("%s", S.c_str());

    if (ArgCnt > 0) {
        AnsiChar Sep = '(';

        for (int j = 0; j < ArgCnt; ++j) {
            OutS += String(Sep);
            OutLog2("%c", Sep);

            switch (Args[j]->Kind) {
                case 0: {
                    Args[j]->C.Show(false);
                    break;
                }

                case 1: {
                    OutS += "TypeInfo(";
                    OutLog1("TypeInfo(");

                    NP = GetTypeName(Args[j]->A.hDT);
                    if (NP) {
                        AnsiString TypeName = NP->GetStr();
                        OutS += TypeName;
                        OutLog2("%s", TypeName.c_str());
                    }

                    OutS += ")";
                    OutLog1(")");
                    break;
                }
            }

            Sep = ',';
        }

        OutS += ")";
        OutLog1(")");
    }

    OutS += "]";
    OutLog1("]\n");
}

//------------------------------------------------------------------------------
bool TAttributeDeclModifier::ShowBefore() { return true; }
//------------------------------------------------------------------------------
TGeneratedNameDeclModifier::TGeneratedNameDeclModifier(TMemStrRef *AMsg) : TStrDeclModifier(AMsg) {}
//------------------------------------------------------------------------------

/**
 * .Net information (was observed in DCUIL but may be used somewhere else)
 *
 * @param OutS
 */
void TGeneratedNameDeclModifier::Show(String &OutS) {
    // todo: review
    OutS = "generated_name ";
    OutS += FMsg->S();
    OutLog2("generated_name %s\n", AnsiString(FMsg->S()).c_str());
}
//------------------------------------------------------------------------------
TExtraArgsDeclModifier::TExtraArgsDeclModifier() : TDeclModifier() {
    ArgCnt = ReadUIndex();
    Args   = new TExtraProcArgs[ArgCnt * sizeof(TExtraProcArg)];
    for (int j = 0; j < ArgCnt; ++j) {
        Args[j]->Name = ReadNDXStrRef();
        Args[j]->V    = ReadUIndex();
        Args[j]->V1   = ReadUIndex();
        Args[j]->hDT  = ReadUIndex();
    }
}
//------------------------------------------------------------------------------
TExtraArgsDeclModifier::~TExtraArgsDeclModifier() {
    if (Args) delete[] Args;
}
//------------------------------------------------------------------------------
/*void TExtraArgsDeclModifier::Read() {
    ArgCnt = ReadUIndex();
    Args   = new TExtraProcArgs[ArgCnt * sizeof(TExtraProcArg)];
    for (int j = 0; j < ArgCnt; ++j) {
        Args[j]->Name = ReadNDXStrRef();
        Args[j]->V    = ReadUIndex();
        Args[j]->V1   = ReadUIndex();
        Args[j]->hDT  = ReadUIndex();
    }
}*/
//------------------------------------------------------------------------------
void TExtraArgsDeclModifier::Show(String &OutS) {
    String S;
    OutS = "extra_parameters";
    OutLog1("extra_parameters");
    if (ArgCnt > 0) {
        for (int j = 0; j < ArgCnt; j++) {
            OutS += "(";
            OutLog1("(");
            if (j > 0) OutLog1("\n");
            S = Args[j]->Name->S();
            OutS += S;
            OutLog2("%s", S.c_str());
            OutLog3("(V:#%x,V1:#%x): ", Args[j]->V, Args[j]->V1);
            S = ShowTypeName(Args[j]->hDT);
            OutS += S;
            OutLog2("%s", S.c_str());
            OutS += ";";
            OutLog1(";");
        }
        OutS += ")";
        OutLog1(")");
    }
}
//------------------------------------------------------------------------------
TTemplateParmsDeclModifier::TTemplateParmsDeclModifier() : TDeclModifier() {
    // The list of the template formal parameters of the data type or procedure
    Cnt = ReadUIndex();
    Tbl = static_cast<PNDXTbl>(AllocMem(Cnt * sizeof(TNDX)));
    for (int i = 0; i < Cnt; ++i) {
        (*Tbl)[i] = ReadUIndex();

        if (FVer >= verD12 && FVer < verK1) {
            ReadSomeNameInfo28();
        }
    }
}
//------------------------------------------------------------------------------
TTemplateParmsDeclModifier::~TTemplateParmsDeclModifier() {
    if (Tbl) {
        delete[] Tbl;
        Tbl = nullptr;
    }
}
//------------------------------------------------------------------------------
void TTemplateParmsDeclModifier::Read(TDCURec *Owner) {
    int hDT = ReadUIndex();

    TTemplateParmsDeclModifier *ParmInf = new TTemplateParmsDeclModifier();

    ParmInf->hFn = hDT;

    if (hDT != 0) {
        Owner = static_cast<TDCURec *>(GetLastAddedTypeDef());
    }

    if (TTypeDef *TypeDefOwner = dynamic_cast<TTypeDef *>(Owner)) {
        TypeDefOwner->AddModifier(ParmInf);
    } else if (TNameDecl *NameDeclOwner = dynamic_cast<TNameDecl *>(Owner)) {
        NameDeclOwner->AddModifier(ParmInf);
    } else {
        printf("[Warning] TTemplateParmsDeclModifier::Read: The type #%x is a %s and not a TTypeDef", hDT, AnsiString(Owner->ClassName()).c_str()); // DCUWarningFmt
        delete ParmInf;
    }
}
//------------------------------------------------------------------------------
void TTemplateParmsDeclModifier::Show(String &OutS) {}
//------------------------------------------------------------------------------
bool TTemplateParmsDeclModifier::ShowBefore() { return TDeclModifier::ShowBefore(); }
//------------------------------------------------------------------------------
/**
 * TNameDecl.Create;
 */
TNameDecl::TNameDecl() : TDCURec() {
    hDecl   = AddAddrDef(this);
    Def     = reinterpret_cast<PNameDef>(DefStart);
    PName N = ReadName();
}
//------------------------------------------------------------------------------
/**
 * TNameDecl.Create0
 * TNameDecl.Create00 (when All=false)
 */
TNameDecl::TNameDecl(bool All) : TDCURec() {
    hDecl = AddAddrDef(this);
    if (All) {
        Def     = reinterpret_cast<PNameDef>(DefStart);
        PName N = ReadName();
        // !!!
        // if (strlen("TAction") == N->Len && !memcmp(N->GetStr(), "TAction", strlen("TAction"))) N = N;
    }
}
//------------------------------------------------------------------------------
TNameDecl::~TNameDecl() {
    ClearAddrDef(this);
    delete FModifiers;
}
//------------------------------------------------------------------------------
void TNameDecl::Visit(TDCURecVisitor *Visitor) { Visitor->visitNameDecl(this); }
//------------------------------------------------------------------------------
void __fastcall TNameDecl::ShowName(String &OutS) {
    // DCU32: PutDCURecStr(Self,hDecl,false);
    OutS = GetDCURecStr(this, hDecl);
    OutLog2("%s", OutS.c_str());
    // if (SameText(OutS, "TAction")) OutS = "TAction";
}
//------------------------------------------------------------------------------
void __fastcall TNameDecl::Show(String &OutS) { ShowName(OutS); }
//------------------------------------------------------------------------------
// todo:
void TNameDecl::ShowConstAddInfo(String &OutS) {
    const int symDeprecated = 0x1;
    const int symPlatform   = 0x2;
    const int symLibrary    = 0x4;
    const int symInline     = 0x80000;

    ShowModifiers(false);
    if (ConstAddInfoFlags == 0) return;

    // The newer versions use modifier for deprecated
    if ((ConstAddInfoFlags & symDeprecated) != 0 && ((FVer >= verD6 && FVer < verD2009) || FVer >= verK1)) {
        // PutSpace;
        // PutKW('deprecated');
        OutS += " deprecated";
    }
    if ((ConstAddInfoFlags & symPlatform) != 0) {
        // PutSpace;
        // PutKW('platform');
        OutS += " platform";
    }
    if ((ConstAddInfoFlags & symLibrary) != 0) {
        // PutSpace;
        // PutKW('library');
        OutS += " library";
    }
}
//------------------------------------------------------------------------------
void __fastcall TNameDecl::ShowDef(bool All, String &OutS) {
    ShowModifiers(true);
    // MarkDefStart(hDecl);
    Show(OutS);
    // ShowConstAddInfo;
}
//------------------------------------------------------------------------------
/**
 * Get Exportable name
 * @return
 */
PName TNameDecl::GetExpName() {
    if (!Def) return &NoName;
    return &Def->Name;
}
//------------------------------------------------------------------------------
PName __fastcall TNameDecl::GetName() {
    // if (!Def) return &NoName;
    // return &Def->Name;

    return GetExpName();
}
//------------------------------------------------------------------------------
DWord __fastcall TNameDecl::SetMem(DWord MOfs, DWord MSz) { return 0; }
//------------------------------------------------------------------------------
Byte __fastcall TNameDecl::GetSecKind() { return skNone; }
//------------------------------------------------------------------------------
bool __fastcall TNameDecl::IsVisible(Byte LK) { return true; }
//------------------------------------------------------------------------------
TDCURecTag __fastcall TNameDecl::GetTag() {
    return FixTag(Def->Tag);
}
//------------------------------------------------------------------------------
void __fastcall TNameDecl::AddModifier(TDeclModifier *M) {
    TDeclModifier *MP = FModifiers;
    while (MP) {
        MP = MP->Next;
    }
    MP = M;
}
//------------------------------------------------------------------------------
void __fastcall TNameDecl::ShowModifiers(bool Before) {
    TDeclModifier *M = FModifiers;
    while (M) {
        String OutS;
        if (M->ShowBefore() == Before) M->Show(OutS);
        M = M->Next;
    }
}
//------------------------------------------------------------------------------
TDeclModifier *TNameDecl::GetModifierOfClass(TDeclModifierClass *Cl) {
    return GetDeclModifierOfClass(FModifiers, Cl);
}
//------------------------------------------------------------------------------
TNameFDecl::TNameFDecl(bool NoInf) : TNameDecl(true) {
    F = ReadUIndex();
    if (FVer == verD6)
        ConstAddInfoFlags = (F >> 9) & 0x07; // Deprecated,Platform,Library were introduced in this version and were stored here
    if (FVer >= verD8 && FVer < verK1)
        F1 = ReadUIndex();
    if (FVer >= verD2009 && FVer < verK1)
        int F4 = ReadUIndex();
    if (!NoInf && (F & 0x40) != 0)
        Inf = ReadULong();

    PkgNdx = -1;

    if (FVer >= verD8 && FVer < verK1) {
        // Could be valid for MSIL only
        if ((F1 & 0x80) != 0) {
            B2 = ReadUIndex();
            if (FVer == verD8 && (F & 0x8) != 0)
                int F3 = ReadUIndex();
        }
    }
}
//------------------------------------------------------------------------------
void __fastcall TNameFDecl::Visit(TDCURecVisitor *Visitor) { Visitor->visitNameFDecl(this); }
//------------------------------------------------------------------------------
void __fastcall TNameFDecl::Show(String &OutS) {
    TNameDecl::Show(OutS);
    // if (PkgNdx >= 0) PutSFmt(',Pkg:%x',[PkgNdx]);
}
//------------------------------------------------------------------------------
bool __fastcall TNameFDecl::IsVisible(Byte LK) {
    if (LK == dlMain) return (F & 0x40) != 0;
    if (LK == dlMainImpl) return (F & 0x40) == 0;
    return true;
}
//------------------------------------------------------------------------------
void __fastcall TNameFDecl::ShowStamps() {}
//------------------------------------------------------------------------------
/**
 * It looks like that the field is processed AFTER reading
 * some data in the ancestor classes
 */
void TNameFDecl::ReadPkgNdx() {
    int M;
    if (FromPackage && ((FVer >= verD3 && FVer <= verD8) || FVer >= verK1)) {
        if (FVer < verD6) M = 0x80;
        else M = 0x100;

        if ((F & M) != 0)
            PkgNdx = ReadUIndex();
    }
}
//------------------------------------------------------------------------------
TTypeDecl::TTypeDecl() : TNameFDecl(false) {
    hDef = ReadUIndex();
    if (FVer >= verD8 && FVer < verK1 && B2) hDef = B2;
    AddTypeName(hDef, hDecl, &Def->Name);
}
//------------------------------------------------------------------------------
/**
 *
 * @param LK TDeclListKind
 * @return
 */
bool __fastcall TTypeDecl::IsVisible(Byte LK) {
    auto Result = TNameFDecl::IsVisible(LK);
    // return TNameFDecl::IsVisible(LK);
    if (!Result)
        return false;
    if (!Def)
        return false;

    PName RefName = &Def->Name;
    AnsiChar ch = RefName->Get1stChar();

    Result = !(ch == '.' || ch == ':') && (FVer >= verD2009 && FVer < verK1);
    return Result;
}
//------------------------------------------------------------------------------
void TTypeDecl::Visit(TDCURecVisitor *Visitor) { Visitor->visitTypeDecl(this); }
//------------------------------------------------------------------------------
void __fastcall TTypeDecl::Show(String &OutS) {
    String S;
    PName RefName;

    OutS = "";

    PTYPEINFO typeInfo   = new TYPEINFO;
    typeInfo->Size       = GetTypeSize(hDef);
    typeInfo->ModuleID   = ModuleID;
    typeInfo->Kind       = 'Z'; // drAlias
    typeInfo->VMCnt      = 0;
    typeInfo->RTTIOfs    = 0xFFFFFFFF;
    typeInfo->RTTISz     = 0;
    typeInfo->Fixups     = new TList;
    typeInfo->Fields     = new TList;
    typeInfo->Properties = new TList;
    typeInfo->Methods    = new TList;
    TNameFDecl::Show(S);
    typeInfo->Name = S;

    OutS += S;

    if (!Def) {
        RefName = nullptr;
    } else {
        // @Def^. changed for templates in Ver>=verD12
        RefName = &Def->Name;
    }

    // dcu32:
    // D := CurUnit.GetLocalTypeDef(hDef);
    // if D<>Nil then
    //   D.ShowModifiers(true{Before});

    OutS += "=";
    OutLog1("=");

    TBaseDef *D = GetTypeDef(hDef);
    if (D) {
        if (D->InheritsFrom((__classid(TArrayDef))))
            typeInfo->Kind = drArrayDef;
        else if (D->InheritsFrom((__classid(TClassDef)))) {
            typeInfo->Kind  = drClassDef;
            typeInfo->VMCnt = static_cast<TClassDef *>(D)->VMCnt;
            FieldsList      = typeInfo->Fields;
            PropertiesList  = typeInfo->Properties;
            MethodsList     = typeInfo->Methods;
        } else if (D->InheritsFrom((__classid(TFileDef))))
            typeInfo->Kind = drFileDef;
        else if (D->InheritsFrom((__classid(TFloatDef))))
            typeInfo->Kind = drFloatDef;
        else if (D->InheritsFrom((__classid(TInterfaceDef)))) {
            typeInfo->Kind  = drInterfaceDef;
            typeInfo->VMCnt = static_cast<TInterfaceDef *>(D)->VMCnt;
            FieldsList      = typeInfo->Fields;
            PropertiesList  = typeInfo->Properties;
            MethodsList     = typeInfo->Methods;
        } else if (D->InheritsFrom((__classid(TObjVMTDef))))
            typeInfo->Kind = drObjVMTDef;
        else if (D->InheritsFrom((__classid(TProcTypeDef))))
            typeInfo->Kind = drProcTypeDef;
        else if (D->InheritsFrom((__classid(TPtrDef))))
            typeInfo->Kind = drPtrDef;
        else if (D->InheritsFrom((__classid(TRangeBaseDef))))
            typeInfo->Kind = drRangeDef;
        else if (D->InheritsFrom((__classid(TRecDef)))) {
            typeInfo->Kind = drRecDef;
            FieldsList     = typeInfo->Fields;
        } else if (D->InheritsFrom((__classid(TSetDef))))
            typeInfo->Kind = drSetDef;
        else if (D->InheritsFrom((__classid(TShortStrDef))))
            typeInfo->Kind = drShortStrDef;
        else if (D->InheritsFrom((__classid(TStringDef))))
            typeInfo->Kind = drStringDef;
        else if (D->InheritsFrom((__classid(TTextDef))))
            typeInfo->Kind = drTextDef;
        else if (D->InheritsFrom((__classid(TVariantDef))))
            typeInfo->Kind = drVariantDef;
    }

    pDumpOffset    = &typeInfo->RTTIOfs;
    pDumpSize      = &typeInfo->RTTISz;
    FixupsList     = typeInfo->Fixups;
    S              = ShowTypeDef(hDef, RefName);

    pDumpOffset    = 0;
    pDumpSize      = 0;
    FixupsList     = 0;
    FieldsList     = 0;
    PropertiesList = 0;
    MethodsList    = 0;

    typeInfo->Decl = S;
    OutS += S;

    if (TypeList) {
        TypeList->Add(static_cast<void *>(typeInfo));
    } else {
        delete typeInfo->Fixups;
        delete typeInfo->Fields;
        delete typeInfo->Properties;
        delete typeInfo->Methods;
        delete typeInfo;
    }
}
//------------------------------------------------------------------------------
void __fastcall TTypeDecl::EnumUsedTypes(TTypeUseAction Action, DWord *IP) {
    TBaseDef *D = GetTypeDef(hDef);
    if (!D || !(D->InheritsFrom(__classid(TTypeDef)))) return;
    if (static_cast<TTypeDef *>(D)->hAddrDef != hDecl)
        Action(this, hDef, IP);
    else
        D->EnumUsedTypes(Action, IP);
}
//------------------------------------------------------------------------------
PName __fastcall TTypeDecl::GetName() {
    // The template name could be fixed
    if (FVer >= verD2009 && FVer < verK1) {
        TTypeDef *TD = GetLocalTypeDef(hDef);
        if (TD && TD->hDecl == hDecl) return TD->FName;
    }
    return TNameDecl::GetName();
}
//------------------------------------------------------------------------------
DWord __fastcall TTypeDecl::SetMem(DWord MOfs, DWord MSz) {
    TTypeDef *D = GetTypeDef(hDef);
    if (!D) return 0;
    return D->SetMem(MOfs, MSz);
}
//------------------------------------------------------------------------------
Byte __fastcall TTypeDecl::GetSecKind() {
    return skType;
}

//------------------------------------------------------------------------------
void TTypeDecl::ShowStamps() { TNameFDecl::ShowStamps(); }
//------------------------------------------------------------------------------
TVarDecl::TVarDecl() : TNameFDecl(false) {
    hDT = ReadUIndex();
    Ofs = ReadUIndex();
}
//------------------------------------------------------------------------------
void TVarDecl::Visit(TDCURecVisitor *Visitor) { Visitor->visitVarDecl(this); }
//------------------------------------------------------------------------------
/*
procedure TVarDecl.Show;
begin
  inherited Show;
  PutS(': ');
  CurUnit.ShowTypeDef(hDT,Nil);
  PutSFmtRemAux('Ofs:$%x',[Ofs]);
end ;
 */
void __fastcall TVarDecl::Show(String &OutS) {
    String S;

    PVARINFO vInfo  = new VARINFO;
    vInfo->ModuleID = ModuleID;
    vInfo->Type     = VI_VAR;
    if (ThreadVar) vInfo->Type = VI_THREADVAR;
    vInfo->DumpOfs = 0xFFFFFFFF;
    vInfo->DumpSz  = 0;
    vInfo->AbsName = "";

    TNameFDecl::Show(S);
    vInfo->Name = S;
    OutS = S + ":";
    OutLog1(":");
    pDumpOffset = &vInfo->DumpOfs;
    pDumpSize = &vInfo->DumpSz;
    S = ShowTypeDef(hDT, nullptr);
    pDumpOffset = nullptr;
    pDumpSize = nullptr;

    vInfo->TypeDef = S;
    OutS += S;
    VarList->Add(static_cast<void *>(vInfo));
}
//------------------------------------------------------------------------------
void __fastcall TVarDecl::EnumUsedTypes(TTypeUseAction Action, DWord *IP) { Action(this, hDT, IP); }
//------------------------------------------------------------------------------
Byte __fastcall TVarDecl::GetSecKind() { return skVar; }
//------------------------------------------------------------------------------
TVarVDecl::TVarVDecl() { Sz = static_cast<DWord>(-1); }
//------------------------------------------------------------------------------
void TVarVDecl::Visit(TDCURecVisitor *Visitor) { Visitor->visitVarVDecl(this); }
//------------------------------------------------------------------------------
void TVarVDecl::Show(String &OutS) {
    DWord DS;
    TFixupMemState MS;
    String S;

    TVarDecl::Show(S);

    if (Sz == static_cast<DWord>(-1)) return;

    OutLog1("=");
    if (Sz == static_cast<DWord>(-1)) {
        // constInfo->Value = "?";
        OutLog1("?");
    } else {
        Byte *DP = GetBlockMem(Ofs, Sz, &DS);
        if (DP) {
            SaveFixupMemState(&MS);
            SetCodeRange(FDataBlPtr, DP, DS);
            int Fix0 = GetStartFixup(Ofs);
            SetStartFixupInfo(Fix0);
        }
        ShowGlobalTypeValue(hDT, DP, DS, true, -1, false, S);
        // constInfo->Value = S;
        if (DP) RestoreFixupMemState(&MS);
    }
}
//------------------------------------------------------------------------------
DWord TVarVDecl::SetMem(DWord MOfs, DWord MSz) {
    DWord Result = 0;
    if (Sz == static_cast<DWord>(-1)) {
        Sz = MSz;
    } else if (Sz != MSz) {
        // Changed for StrConstRec
        printf("[Error] Trying to change variable %s{$%x} memory to $%x[$%x]\n", Name->GetStr().c_str(), hDecl, MOfs, MSz); // DCUErrorFmt
    }

    if (Ofs == static_cast<DWord>(-1)) {
        Ofs = MOfs;
    } else if (Ofs != MOfs) {
        printf("[Error] variable %s{$%x}: memory ofs mismatch ($%x<>$%x)\n", Name->GetStr().c_str(), hDecl, Ofs, MOfs); // DCUErrorFmt
    }

    return Result;
}
//------------------------------------------------------------------------------
void TVarCDecl::SetPDataLinks() {
    TWin64UnwindInfo *UnwindInfo = new TWin64UnwindInfo;
    if (UnwindInfo->InitPData(hDecl)) UnwindInfo->SetPDataLinks(hDecl);
}
//------------------------------------------------------------------------------
bool TVarCDecl::IsSpecialConst() {
    TTypeDef *DT = GetLocalTypeDef(hDT);
    return ((DT->ClassType() == __classid(TVoidDef)) && (DT->Sz == Sz));
}
//------------------------------------------------------------------------------
void TVarCDecl::SetSegKind(TSegKind V) {
    FSeg = V;
    if (FPlatform == dcuplWin64 && FSeg == TSegKind::PData && IsSpecialConst())
        SetPDataLinks();
}
//------------------------------------------------------------------------------
TVarCDecl::TVarCDecl(bool OfsValid) : TVarDecl() {
    Sz   = static_cast<DWord>(-1);
    OfsR = Ofs;
    if (!OfsValid) Ofs = static_cast<DWord>(-1);
}
//------------------------------------------------------------------------------
void TVarCDecl::Visit(TDCURecVisitor *Visitor) { Visitor->visitVarCDecl(this); }
//------------------------------------------------------------------------------
int       CodeFixupCnt;
PFixupRec CodeFixups;
Byte     *CodeBase;
Byte     *CodeEnd;
Byte     *CodeStart;
Byte     *FixUpEnd;

void __fastcall ClearFixupInfo() {
    CodeFixupCnt = 0;
    CodeFixups   = nullptr;
}
//------------------------------------------------------------------------------
void __fastcall SetFixupInfo(int ACodeFixupCnt, PFixupRec ACodeFixups) {
    CodeFixupCnt = ACodeFixupCnt;
    CodeFixups = ACodeFixups;
}
//------------------------------------------------------------------------------
void __fastcall SetCodeRange(Byte* ACodeStart, Byte* ACodeBase, DWord ABlSz) {
    ClearFixupInfo();
    CodeStart = ACodeStart;
    CodeBase = ACodeBase;
    CodeEnd = CodeBase + ABlSz;
    FixUpEnd = CodeBase;
}
//------------------------------------------------------------------------------
void __fastcall SaveFixupState(TFixupState* S) {
    S->FixCnt = CodeFixupCnt;
    S->Fix = CodeFixups;
    S->FixEnd = FixUpEnd;
}
//------------------------------------------------------------------------------
void __fastcall RestoreFixupState(TFixupState* S) {
    CodeFixupCnt = S->FixCnt;
    CodeFixups = S->Fix;
    FixUpEnd = S->FixEnd;
}
//------------------------------------------------------------------------------
void __fastcall SaveFixupMemState(TFixupMemState* S) {
    SaveFixupState(&S->Fx);
    S->CodeBase = CodeBase;
    S->CodeEnd = CodeEnd;
    S->CodeStart = CodeStart;
}
//------------------------------------------------------------------------------
void __fastcall RestoreFixupMemState(TFixupMemState* S) {
    RestoreFixupState(&S->Fx);
    CodeBase = S->CodeBase;
    CodeEnd = S->CodeEnd;
    CodeStart = S->CodeStart;
}
//------------------------------------------------------------------------------
void __fastcall SetStartFixupInfo(int Fix0) {
    SetFixupInfo(FFixupCnt - Fix0, &FFixupTbl[Fix0]);
}
//------------------------------------------------------------------------------
void __fastcall TVarCDecl::Show(String &OutS) {
    DWord          DS;
    TFixupMemState MS;
    String         S;

    PCONSTINFO constInfo = new CONSTINFO;
    constInfo->ModuleID  = ModuleID;
    constInfo->Type      = CI_VARCDECL;
    constInfo->TypeDef   = "";
    constInfo->Value     = "";
    constInfo->RTTIOfs   = 0xFFFFFFFF;
    constInfo->RTTISz    = 0;
    constInfo->Fixups    = new TList;

    pDumpOffset = &constInfo->RTTIOfs;
    pDumpSize   = &constInfo->RTTISz;
    FixupsList  = constInfo->Fixups;

    TNameFDecl::Show(S);
    constInfo->Name = S;
    OutLog1(":");
    constInfo->TypeDef = ShowTypeDef(hDT, nullptr);
    OutLog1("=");

    if (Sz == static_cast<DWord>(-1)) {
        constInfo->Value = "?";
        OutLog1("?");
    } else {
        Byte *DP = GetBlockMem(Ofs, Sz, &DS);
        if (DP) {
            SaveFixupMemState(&MS);
            SetCodeRange(FDataBlPtr, DP, DS);
            int Fix0 = GetStartFixup(Ofs);
            SetStartFixupInfo(Fix0);
        }
        ShowGlobalTypeValue(hDT, DP, DS, true, -1, true, S);
        constInfo->Value = S;
        if (DP) RestoreFixupMemState(&MS);
    }

    pDumpOffset = 0;
    pDumpSize   = 0;
    FixupsList  = 0;

    if (ConstList)
        ConstList->Add(static_cast<void *>(constInfo));
    else {
        delete constInfo->Fixups;
        delete constInfo;
    }
    OutS = "";
}
//------------------------------------------------------------------------------
DWord __fastcall TVarCDecl::SetMem(DWord MOfs, DWord MSz) {
    if (Sz == static_cast<DWord>(-1)) Sz = MSz;
    if (Ofs == static_cast<DWord>(-1)) Ofs = MOfs;
    return 0;
}
//------------------------------------------------------------------------------
Byte __fastcall TVarCDecl::GetSecKind() {
    if (GenVarCAsVars)
        return skVar;
    else
        return skConst;
}
//------------------------------------------------------------------------------
TAbsVarDecl::TAbsVarDecl() : TVarDecl() {
    RefAddrDef(Ofs); // forward references could happen e.g. by referencing Self in embedded proc
}
//------------------------------------------------------------------------------
void TAbsVarDecl::Visit(TDCURecVisitor *Visitor) { Visitor->visitAbsVarDecl(this); }
//------------------------------------------------------------------------------
void __fastcall TAbsVarDecl::Show(String &OutS) {
    String S;

    PVARINFO vInfo = new VARINFO;
    vInfo->ModuleID = ModuleID;
    vInfo->Type     = VI_ABSVAR;
    vInfo->DumpOfs  = 0xFFFFFFFF;
    vInfo->DumpSz   = 0;

    TNameFDecl::Show(S);
    vInfo->Name = S;
    OutS = S + ":";
    OutLog1(":");
    pDumpOffset = &vInfo->DumpOfs;
    pDumpSize = &vInfo->DumpSz;
    S = ShowTypeDef(hDT, nullptr);
    pDumpOffset = 0;
    pDumpSize = 0;

    vInfo->TypeDef = S;
    OutS += S;

    S = GetAddrStr(static_cast<int>(Ofs), false);
    vInfo->AbsName = S;
    OutS += " absolute " + S;
    OutLog2(" absolute %s", S.c_str());

    VarList->Add(static_cast<void *>(vInfo));
}
//------------------------------------------------------------------------------
TTypePDecl::TTypePDecl() : TVarCDecl(false) {}
//------------------------------------------------------------------------------
void TTypePDecl::Visit(TDCURecVisitor *Visitor) { Visitor->visitTypePDecl(this); }
//------------------------------------------------------------------------------
void __fastcall TTypePDecl::Show(String &OutS) {
    DWord DS;
    TFixupMemState MS;
    String S;

    PCONSTINFO constInfo = new CONSTINFO;
    constInfo->ModuleID  = ModuleID;
    constInfo->Type      = CI_PDECL;
    constInfo->TypeDef   = "";
    constInfo->Value     = "";
    constInfo->RTTIOfs   = 0xFFFFFFFF;
    constInfo->RTTISz    = 0;
    constInfo->Fixups    = new TList;

    OutLog1("VMT ");
    pDumpOffset = &constInfo->RTTIOfs;
    pDumpSize   = &constInfo->RTTISz;
    FixupsList  = constInfo->Fixups;

    TNameFDecl::Show(S);
    constInfo->Name = S;
    OutLog1(":");
    constInfo->TypeDef = ShowTypeDef(hDT, NULL);
    OutLog1("=");

    if (Sz == static_cast<DWord>(-1)) {
        constInfo->Value = "?";
        OutLog1("?");
    } else {
        Byte *DP = GetBlockMem(Ofs, Sz, &DS);
        if (DP) {
            SaveFixupMemState(&MS);
            SetCodeRange(FDataBlPtr, DP, DS);
            int Fix0 = GetStartFixup(Ofs);
            SetStartFixupInfo(Fix0);
        }
        ShowGlobalTypeValue(hDT, DP, DS, true, -1, false, S);
        constInfo->Value = S;
        if (DP) RestoreFixupMemState(&MS);
    }

    pDumpOffset = 0;
    pDumpSize   = 0;
    FixupsList  = 0;

    if (ConstList)
        ConstList->Add(static_cast<void *>(constInfo));
    else {
        delete constInfo->Fixups;
        delete constInfo;
    }
    OutS = "";
}
//------------------------------------------------------------------------------
bool __fastcall TTypePDecl::IsVisible(Byte LK) {
    return true;
}
//------------------------------------------------------------------------------
void TTypePDecl::ShowStamps() { TVarCDecl::ShowStamps(); }
//------------------------------------------------------------------------------
TThreadVarDecl::TThreadVarDecl() : TVarDecl() {}
//------------------------------------------------------------------------------
Byte __fastcall TThreadVarDecl::GetSecKind() {
    return skThreadVar;
}
//------------------------------------------------------------------------------
void TThreadVarDecl::Visit(TDCURecVisitor *Visitor) { Visitor->visitThreadVarDecl(this); }
//------------------------------------------------------------------------------
void TMemBlockRef::MemRefFound() {}
//------------------------------------------------------------------------------
void TMemBlockRef::Visit(TDCURecVisitor *Visitor) { Visitor->visitMemBlockRef(this); }
//------------------------------------------------------------------------------
TStrConstDecl::TStrConstDecl() : TMemBlockRef(false) {
    TNDX X;
    if (FVer >= verDXE1 && FVer < verK1) {
        FX = Ofs;
        Ofs = ReadUIndex();
        if (Ofs == 0) Ofs = static_cast<DWord>(-1);
        Sz  = ReadUIndex();
        hDT = -1;
    } else {
        FX = ReadUIndex();
        Ofs = static_cast<DWord>(-1);
        X = ReadUIndex();
        if (IsMSIL) {
            FX1 = X;
            Sz = static_cast<DWord>(-1);
        } else {
            Sz = X;
        }
        hDT = -1;
    }

    if (FVer >= verDXE1 && FVer < verK1) {
        X = ReadByte(); // ReadUIndex() - it was detected in verD_XE2 and Ok for verD_XE
    }
}
//------------------------------------------------------------------------------
DWord __fastcall TStrConstDecl::SetMem(DWord MOfs, DWord MSz) {
    if (Sz == static_cast<DWord>(-1)) Sz = MSz;
    if (Ofs == static_cast<DWord>(-1)) Ofs = MOfs;
    return 0;
}
//------------------------------------------------------------------------------
Byte __fastcall TStrConstDecl::GetSecKind() {
    if (GenVarCAsVars)
        return skVar;
    return skConst;
}
//------------------------------------------------------------------------------
void TStrConstDecl::MemRefFound() { TMemBlockRef::MemRefFound(); }
//------------------------------------------------------------------------------
void TStrConstDecl::Visit(TDCURecVisitor *Visitor) { Visitor->visitStrConstDecl(this); }
//------------------------------------------------------------------------------
void __fastcall TStrConstDecl::Show(String &OutS) {
    DWord          DS;
    TFixupMemState MS;

    TNameFDecl::Show(OutS);
    OutS += ":";
    OutLog1(":");
    OutS += ShowTypeDef(hDT, NULL);
    OutLog1("=");
    if (Sz == static_cast<DWord>(-1)) {
        OutLog1("?");
    } else {
        String S;
        Byte *DP = GetBlockMem(Ofs, Sz, &DS);
        if (DP) {
            SaveFixupMemState(&MS);
            SetCodeRange(FDataBlPtr, DP, DS);
            int Fix0 = GetStartFixup(Ofs);
            SetStartFixupInfo(Fix0);
        }
        ShowGlobalTypeValue(hDT, DP, DS, true, -1, false, S);
        if (DP) RestoreFixupMemState(&MS);
    }
}
//------------------------------------------------------------------------------
void __fastcall TStrConstDecl::EnumUsedTypes(TTypeUseAction Action, DWord *IP) {
    Action(this, hDT, IP);
}
//------------------------------------------------------------------------------
TLabelDecl::TLabelDecl() : TNameDecl(true) {
    Ofs = ReadUIndex();
    if (FVer >= verD8 && FVer < verK1) ReadUIndex(); // =0
    if (FVer >= verD2009 && FVer < verK1) ReadUIndex();
}
//------------------------------------------------------------------------------
void TLabelDecl::Visit(TDCURecVisitor *Visitor) { Visitor->visitLabelDecl(this); }
//------------------------------------------------------------------------------
void __fastcall TLabelDecl::Show(String &OutS) {
    TNameDecl::Show(OutS);
    // DCU32: PutSFmtRem('at $%x',[Ofs]);
}
//------------------------------------------------------------------------------
Byte __fastcall TLabelDecl::GetSecKind() {
    return skLabel;
}
//------------------------------------------------------------------------------
bool __fastcall TLabelDecl::IsVisible(Byte LK) {
    return LK != dlMain;
}
//------------------------------------------------------------------------------
TExportDecl::TExportDecl() : TNameDecl() {
    hSym = ReadUIndex();
    Index = ReadUIndex();
}
//------------------------------------------------------------------------------
void TExportDecl::Visit(TDCURecVisitor *Visitor) { Visitor->visitExportDecl(this); }
//------------------------------------------------------------------------------
void __fastcall TExportDecl::Show(String &OutS) {
    TDCURec *D = GetAddrDef(hSym);
    PName    N = nullptr;

    if (!D) {
        OutLog1("?");
    } else {
        D->ShowName(OutS);
        N = D->Name;
    }

    // was:
    // String sN    = PName2String(N);
    // String sName = PName2String(Name);

    if (N && Name && !N->Eq(Name)) {
        AnsiString sName = Ansistrings::AnsiQuotedStr(Name->GetStr(), '\'');
        OutLog1(" name ");
        // DCU32: PutAddrDefStr(sName,hDecl); // todo?
        ShowName(OutS); // todo? comment this out?
    }
    if (Index) {
        OutLog2(" index %lX", Index);
    }
}
//------------------------------------------------------------------------------
Byte __fastcall TExportDecl::GetSecKind() { return skExport; }
//------------------------------------------------------------------------------
bool TExportDecl::IsVisible(Byte LK) { return LK != dlMain; /* let`s show everything in implementation */ }
//------------------------------------------------------------------------------
TLocalDeclBase::TLocalDeclBase() : TNameDecl() {
    LocFlags = ReadUIndex();

    if (FVer >= verD8 && FVer < verK1) {
        LocFlagsX = ReadUIndex();
        // To make the constants compatible with the previous versions
        LocFlagsX = ((LocFlagsX & ~lfClassV8up ) << 1) | ((LocFlagsX & lfClassV8up) >> 4);
    } else {
        LocFlagsX = LocFlags; // To simplify the rest of the code
    }
    // Just in case - it should be 0 anyway
    LocFlagsX &= ~lfauxPropField;

    if (FVer >= verD2009 && FVer < verK1)
        ReadUIndex(); // B3
}
//------------------------------------------------------------------------------
Byte __fastcall TLocalDeclBase::GetLocFlagsSecKind() {
    switch (LocFlags & lfScope) {
        case lfPrivate: return skPrivate;
        case lfProtected: return skProtected;
        case lfPublic: return skPublic;
        case lfPublished: return skPublished;
    }
    return skNone;
}
//------------------------------------------------------------------------------
/**
 *
 * @param LK List Kind (TDeclListKind)
 */
TLocalDecl::TLocalDecl(Byte LK) : TLocalDeclBase() {
    if (FVer >= verDXE4 && FVer < verK1 && (LK == dlArgs || LK == dlArgsT) && ((LocFlags & 0x40) != 0))
        ReadULong(); // it was observed after the [REF] decorator

    TDCURecTag tg  = TNameDecl::GetTag();

    bool M   = (tg == arMethod || tg == arConstr || tg == arDestr);
    bool M2  = ((FVer == verD2) & M);
    LocFlags = ReadUIndex();

    /*if (FVer >= verD8 && FVer < verK1) {
        LocFlagsX = ReadUIndex();
        // To make the constants compatible with the previous versions
        LocFlagsX = ((LocFlagsX & ~lfClassV8up) << 1) | ((LocFlagsX & lfClassV8up) >> 4);
    } else
        LocFlagsX = LocFlags; // To simplify the rest of the code*/

    // LocFlagsX &= ~lfauxPropField;
    // if (FVer >= verD2009 && FVer < verK1) ReadUIndex();

    if (!M2) {
        hDT = ReadUIndex();
        if (M)
            Ndx = ReadIndex();
        else
            Ndx = ReadUIndex();
    } else if (M)
        Ndx = ReadUIndex();
    else
        Ndx = ReadIndex();

    if (LK == dlInterface || LK == dlDispInterface)
        NdxB = ReadUIndex();
    else
        NdxB = -1;

    if (!M2) {
        if (M)
            Ndx = ReadUIndex();
        else
            Ndx = ReadIndex();
    } else
        hDT = ReadIndex(); // ReadUIndex()

    if (LK != dlClass && LK != dlInterface && LK != dlDispInterface && LK != dlFields) {
        tg = GetTag();
        if (tg == arFld || tg == arMethod || tg == arConstr || tg == arDestr) return;
    }
    if (GetTag() == arAbsLocVar)
        RefAddrDef(Ndx); // forward references could happen e.g. by referencing Self in embedded proc
}
//------------------------------------------------------------------------------
void TLocalDecl::Visit(TDCURecVisitor *Visitor) { Visitor->visitLocalDecl(this); }
//------------------------------------------------------------------------------
String __fastcall TLocalDecl::GetPrefix(bool &IsConst) {
    TDCURecTag Tag = GetTag(); // Byte
    IsConst = false;

    String Result;

    // if (ShowAuxValues) // always true

    switch (Tag) {
        case arVal: Result = "val"; break;
        case arVar: Result = "var"; break;
        case drVar: Result = "local"; break;
        case arResult: Result = "result"; break;
        case arAbsLocVar: Result = "local absolute"; break;
        case arFld: Result = "field"; break;
        // case arMethod: Result = "method"; break;
        // case arConstr: Result = "constructor"; break;
        // case arDestr: Result = "destructor"; break;
    }
    if ((Tag == arVal || Tag == arVar) && (LocFlags & 0x7) == 0x1) {
        IsConst = true;
    }

    return "";
}
//------------------------------------------------------------------------------
String RegName[7] = {"EAX", "EDX", "ECX", "EBX", "ESI", "EDI", "EBP"};

void __fastcall TLocalDecl::Show(String &OutS) {
    TDCURecTag Tg = GetTag();
    String S;

    OutS = "";

    // IDR:
    PLOCALDECLINFO info = new LOCALDECLINFO;
    info->Scope         = ActiveScope;
    info->Tag           = Tg;
    info->LocFlags      = LocFlags;
    info->Ndx           = Ndx;
    info->NdxB          = NdxB;
    info->Case          = CaseN;

    ShowModifiers(true);
    // bool IsConst;
    // MS = GetPrefix(IsConst);
    // if (IsConst) {
    //     MS = "const";
    // }

    AnsiString MS;

    // Part of GetPrefix:
    switch (Tg) {
        case arVal: MS = "val "; break;
        case arVar: MS = "var "; break;
        case drVar: MS = "local "; break;
        case arResult: MS = "result "; break;
        case arAbsLocVar: MS = "local absolute "; break;
        case arFld: MS = "field "; break;
        default: MS = ""; break;
    }

    if (MS != "") {
        OutS += MS;
        OutLog2("%s", MS.c_str());
    }

    TNameDecl::Show(S);
    info->Name = S;
    OutS += S + ":";
    OutLog1(":");

    S = ShowTypeDef(hDT, NULL);

    info->TypeDef = S;
    info->AbsName = "";
    OutS += S;

    if ((LocFlags & 8) != 0 && Tg != arFld) {
        if (Ndx >= 0 && Ndx <= 6) {
            OutLog2("{%s}", RegName[Ndx].c_str());
        } else {
            OutLog1("{?}");
        }
    } else
        OutLog2("{Ofs:%d}", static_cast<int>(Ndx));

    if (Tg == arAbsLocVar) {
        S = GetAddrStr(static_cast<int>(Ndx), false);
        OutS += " absolute " + S;
        OutLog2(" absolute %s", S.c_str());
        info->AbsName = S;
    }

    if (FieldsList)
        FieldsList->Add(static_cast<void *>(info));
    else if (ArgsList)
        ArgsList->Add(static_cast<void *>(info));
    else if (LocalsList)
        LocalsList->Add(static_cast<void *>(info));
    else
        delete info;

    // todo?
    // ShowConstAddInfo(OutS);
}
//------------------------------------------------------------------------------
void __fastcall TLocalDecl::EnumUsedTypes(TTypeUseAction Action, DWord *IP) {
    Action(this, hDT, IP);
}
//------------------------------------------------------------------------------
Byte __fastcall TLocalDecl::GetSecKind() {
    TDCURecTag tg = GetTag();
    if (tg == arFld || tg == arMethod || tg == arConstr || tg == arDestr || tg == arProperty || tg == arClassVar)
        return GetLocFlagsSecKind();
    tg = GetTag();
    if (tg == arResult || tg == drVar || tg == arAbsLocVar) return skVar;
    return skNone;
}
//------------------------------------------------------------------------------
TLocalValDecl::TLocalValDecl(Byte lk) : TLocalDecl(lk) {
    // printf("Debug: TLocalValDecl init\n");
}
//------------------------------------------------------------------------------
void TLocalValDecl::Show(String &OutS) {
    TLocalDecl::Show(OutS);
    if (hDeftVal > 0) {
        OutLog1("=");
        ShowGlobalConstValue(hDeftVal, OutS);
    }
}
//------------------------------------------------------------------------------
TMethodDecl::TMethodDecl(Byte LK) : TLocalDecl(LK) {
    // todo
    const TByteSet cS12  = TByteSet() << 0 << 2 << 4 << 8 << 0x10 << 0x18 << 0x20 << 0x80 << 0x84 << static_cast<int>(' ') << static_cast<int>('!') << static_cast<int>('a');
    const TByteSet cS12a = cS12  + (TByteSet() << 1);
    const TByteSet cS12b = cS12a + (TByteSet() << 0x28 << 0x38);
    const TByteSet cS12c = cS12b + (TByteSet() << 0x42 << 0x22 << 0x9);
    const TByteSet cS17  = cS12c + (TByteSet() << 0x47 << 0x4F);
    const TByteSet cS20  = cS17  + (TByteSet() << 0x60);
    const TByteSet cS21  = cS20  + (TByteSet() << 0xA1);
    const TByteSet cS24  = cS21  + (TByteSet() << 0x7 << 0x41);

    // Array of sets
    const TByteSet sSkip[8] = {
        cS12, cS12a, cS12b, cS12c, cS17, cS20, cS21, cS24
    };

    int nSkip = 0;

    InIntrf = LK == dlInterface || LK == dlDispInterface;
    if (!InIntrf) {
        if (IsMSIL && Ndx)
            ReadByteIfEQ(1); // I was unable to find something less perverse to skip this byte

        PName name = TNameDecl::GetName();

        if (FVer >= verD2009 && FVer < verK1 && name->IsEmpty() ) // && GetTag() != arMethod
            ReadByte();

        if ((FVer >= verD7 && FVer < verK1) || name->IsEmpty())
            hImport = ReadUIndex(); // then hDT seems to be valid index in the parent class unit

        // clever
        /*if (FVer >= verD2009 && FVer < verK1 && GetTag() == arMethod) {
            while (ReadByteFrom(FVer >= verD2010) >= 0);
        }*/

        if (FVer >= verD2009 && FVer < verK1 && GetTag() == arMethod) {
            nSkip = 0;
            if (FVer >= verD2010) {
                nSkip++;
                if (FVer >= verDXE2) {
                    nSkip++;
                    if (FVer >= verDXE3) {
                        nSkip++;
                        if (FVer >= verDXE4) {
                            nSkip++;
                            if (FVer >= verDXE7) {
                                nSkip++;
                                if (FVer >= verDXE8) {
                                    nSkip++;
                                    if (FVer >= verD10_2) nSkip++;
                                }
                            }
                        }
                    }
                }
            }
        }

        while (ReadByteFrom(sSkip[nSkip]) >= 0);
    }
}
//------------------------------------------------------------------------------
void TMethodDecl::Visit(TDCURecVisitor *Visitor) { Visitor->visitMethodDecl(this); }
//------------------------------------------------------------------------------
void __fastcall TMethodDecl::Show(String &OutS) {

    auto ShowFlags = [&]() {
        if (Name && Name->IsEmpty() && hImport != 0) {
            OutS += "hImp: #" + IntToStr(hImport);
        }
    };

    String   MS, PS, S;
    TDCURec *D;
    TDCURecTag Tg = GetTag(); // TMethodKind

    PMETHODDECLINFO MethodDeclInfo = new METHODDECLINFO;
    MethodDeclInfo->Scope          = ActiveScope;

    OutS = "";

    if ((LocFlags & lfClass) != 0) {
        PS = "class ";
        OutS += "class ";
        OutLog1("class ");
    }

    if (Ndx || !IsMSIL) {
        D = GetAddrDef(Ndx);
        if (D && !D->InheritsFrom(__classid(TProcDecl))) D = nullptr;
        if (D) {
            Byte MK;
            switch (Tg) {
                case arMethod: MK = mkMethod; break;
                case arConstr: MK = mkConstructor; break;
                case arDestr:  MK = mkDestructor; break;
            }
            static_cast<TProcDecl *>(D)->MethodKind = MK;
        }
    }

    switch (Tg) {
        case arMethod:
            if (!D) {
                MS = "method ";
                MethodDeclInfo->MethodKind = 'M';
            } else if (static_cast<TProcDecl *>(D)->IsProc()) {
                MS                         = "procedure ";
                MethodDeclInfo->MethodKind = 'P';
            } else {
                MS                         = "function ";
                MethodDeclInfo->MethodKind = 'F';
            }
            break;
        case arConstr:
            MS  = "constructor ";
            MethodDeclInfo->MethodKind = 'C';
            break;
        case arDestr:
            MS = "destructor ";
            MethodDeclInfo->MethodKind = 'D';
            break;
    }

    MethodDeclInfo->Prototype = "";

    if (!InIntrf && (Ndx || !IsMSIL)) {
        if (MS != "") {
            PS += MS;
            OutS += MS;
            OutLog2("%s", MS.c_str());
        }
        ShowName(S);
        PS += S;
        OutS += S;

        if (!D) {
            OutS += ":";
            PS += ":";
            OutLog1(":");
        }

        ShowFlags();

        if (D)
            static_cast<TProcDecl *>(D)->ShowArgs(S, 0);
        else {
            S = GetAddrStr(Ndx, false);
            OutLog2("%s", S.c_str());
        }

        PS += S;
        OutS += S;

        if ((LocFlags & lfOverride) != 0) {
            PS += ";override";
            OutS += ";override";
            OutLog1(";override{");
            if ((LocFlags & lfVirtual) != 0) OutLog1(";virtual");
            if ((LocFlags & lfDynamic) != 0) OutLog1(";dynamic");
            OutLog1("}");
        } else {
            if ((LocFlags & lfVirtual) != 0) {
                PS += ";virtual";
                OutS += ";virtual";
                OutLog1(";virtual");
            }
            if ((LocFlags & lfDynamic) != 0) {
                PS += ";dynamic";
                OutS += ";dynamic";
                OutLog1(";dynamic");
            }
        }
        MethodDeclInfo->Prototype = PS;
    } else {
        if (MS != "") {
            PS += MS;
            OutS += MS;
            OutLog2("%s", MS.c_str());
        }

        if (!Ndx && IsMSIL)
            D = GetTypeDef(hImport);
        else
            D = GetTypeDef(Ndx);

        if (D && D->InheritsFrom(__classid(TProcTypeDef))) {
            S = static_cast<TProcTypeDef *>(D)->ProcStr();
            if (S[1] == 'p')
                MethodDeclInfo->MethodKind = 'P';
            else if (S[1] == 'f')
                MethodDeclInfo->MethodKind = 'F';
            PS += S + " ";
            OutS += S + " ";
            OutLog2("%s ", S.c_str());
            ShowName(S);
            PS += S;
            OutS += S;
            static_cast<TProcTypeDef *>(D)->ShowDecl("()", S);
            PS += S;
            OutS += S;
            ShowFlags();
            MethodDeclInfo->Prototype = PS;
        } else {
            ShowName(S);
            PS += S + ":";
            OutS += S + ":";
            OutLog1(": ");
            ShowFlags();
            S = ShowTypeDef(Ndx, GetName());
            PS += S;
            OutS += S;
            MethodDeclInfo->Prototype = PS;
        }
    }
    if (MethodsList) MethodsList->Add(static_cast<void *>(MethodDeclInfo));
}
//------------------------------------------------------------------------------
TClassVarDecl::TClassVarDecl(Byte LK) : TLocalDecl(LK) {}
//------------------------------------------------------------------------------
void TClassVarDecl::Visit(TDCURecVisitor *Visitor) { Visitor->visitClassVarDecl(this); }
//------------------------------------------------------------------------------
void __fastcall TClassVarDecl::Show(String &OutS) {
    OutLog1("class var ");
    TLocalDecl::Show(OutS);
}
//------------------------------------------------------------------------------
Byte __fastcall TClassVarDecl::GetSecKind() {
    return GetLocFlagsSecKind();
}
//------------------------------------------------------------------------------
TPropDecl::TPropDecl() : TLocalDeclBase() {
    LocFlags = ReadIndex();
    if (FVer >= verD8 && FVer < verK1) {
        LocFlagsX = ReadUIndex();
        LocFlagsX = ((LocFlagsX & ~lfClassV8up) << 1) |
                    ((LocFlagsX & lfClassV8up) >> 4); // To make the constants compatible with the previous versions
    } else
        LocFlagsX = LocFlags; // To simplify the rest of the code

    if (FVer >= verD2009 && FVer < verK1) int X4 = ReadUIndex();

    hDT     = ReadUIndex();
    Ndx     = ReadIndex();
    hIndex  = ReadIndex();
    hRead   = ReadUIndex();
    hWrite  = ReadUIndex();
    hStored = ReadUIndex();

    // forward references could happen by mentioning parent methods or fields
    // when defining child properties and when child definition goes before parent in DCU
    // due to usage of TChild = class; before TParent definition
    if (hRead) RefAddrDef(hRead);
    if (hWrite) RefAddrDef(hWrite);
    if (hStored) RefAddrDef(hStored);
    if (FVer >= verD8 && FVer < verK1) {
        int X  = ReadUIndex();
        int X1 = ReadUIndex();
        if (IsMSIL) {
            int X2 = ReadUIndex();
            int X3 = ReadUIndex();
        }
    }
    hDeft = ReadIndex();
}
//------------------------------------------------------------------------------
String __fastcall TPropDecl::PutOp(String Name, int hOp) {
    String V;

    if (!hOp) return "";
    V = GetAddrStr(hOp, false);
    OutLog3(" %s %s", Name.c_str(), V.c_str());
    return V;
}
//------------------------------------------------------------------------------
void TPropDecl::Visit(TDCURecVisitor *Visitor) { Visitor->visitPropDecl(this); }
//------------------------------------------------------------------------------
void __fastcall TPropDecl::Show(String &OutS) {
    String S;
    OutS = "";

    PPROPERTYINFO pInfo = new PROPERTYINFO;
    pInfo->Scope        = ActiveScope;
    pInfo->Index        = hIndex;
    pInfo->DispId       = 0;
    pInfo->TypeDef      = "";
    OutLog1("property ");
    TNameDecl::Show(S);
    pInfo->Name = S;

    if (hDT) {
        TBaseDef *D = GetTypeDef(hDT);
        if (D && D->InheritsFrom(__classid(TProcTypeDef)) && !D->FName) {
            static_cast<TProcTypeDef *>(D)->ShowDecl("[]", OutS);
        } else {
            OutLog1(":");
            pInfo->TypeDef = ShowTypeDef(hDT, NULL);
        }
    }
    if (hIndex != static_cast<int>(0x80000000)) OutLog2("index %lX", hIndex);
    pInfo->ReadName   = PutOp("read", hRead);
    pInfo->WriteName  = PutOp("write", hWrite);
    pInfo->StoredName = PutOp("stored", hStored);

    if (PropertiesList) PropertiesList->Add(static_cast<void *>(pInfo));
}
//------------------------------------------------------------------------------
void __fastcall TPropDecl::EnumUsedTypes(TTypeUseAction Action, DWord *IP) {
    Action(this, hDT, IP);
}
//------------------------------------------------------------------------------
Byte __fastcall TPropDecl::GetSecKind() {
    return GetLocFlagsSecKind();

    /*switch (LocFlags & lfScope) {
        case lfPrivate: return skPrivate;
        case lfProtected: return skProtected;
        case lfPublic: return skPublic;
        case lfPublished: return skPublished;
    }
    return skNone;*/
}
//------------------------------------------------------------------------------
TDispPropDecl::TDispPropDecl(Byte LK) : TLocalDecl(LK) {
}
//------------------------------------------------------------------------------
void TDispPropDecl::Visit(TDCURecVisitor *Visitor) { Visitor->visitDispPropDecl(this); }
//------------------------------------------------------------------------------
void __fastcall TDispPropDecl::Show(String &OutS) {
    String S;

    PPROPERTYINFO pInfo = new PROPERTYINFO;
    pInfo->Scope        = ActiveScope;
    pInfo->Index        = NdxB;
    pInfo->DispId       = Ndx;
    pInfo->ReadName     = "";
    pInfo->WriteName    = "";
    pInfo->StoredName   = "";

    OutLog1("property ");
    ShowName(S);
    pInfo->Name = S;
    OutLog1(": ");
    pInfo->TypeDef = ShowTypeDef(hDT, NULL);

    if (NdxB != -1) {
        switch (NdxB & 6) {
            case 2: OutLog1(" readonly"); break;
            case 4: OutLog1(" writeonly"); break;
        }
    }
    OutLog2(" dispid %lX", static_cast<int>(Ndx));

    if (PropertiesList) PropertiesList->Add(static_cast<void *>(pInfo));

    OutS = "";
}
//------------------------------------------------------------------------------
TConstDeclBase::TConstDeclBase() : TNameFDecl(false) {}
//------------------------------------------------------------------------------
void __fastcall TConstDeclBase::ReadConstVal() {
    Value.ValSz = ReadUIndex();
    if (!Value.ValSz) {
        Value.ValPtr = 0;
        Value.Val    = ReadIndex();
        Value.ValSz  = NDXHi;
    } else {
        Value.ValPtr = CurPos;
        SkipBlock(Value.ValSz);
        Value.Val = 0;
    }
    /*ValSz = ReadUIndex();
    if (!ValSz) {
        ValPtr = 0;
        Val    = ReadIndex();
        ValSz  = NDXHi;
    } else {
        ValPtr = CurPos;
        SkipBlock(ValSz);
        Val = 0;
    }*/
}
//------------------------------------------------------------------------------
void __fastcall TConstDeclBase::ShowValue(String &OutS) {
    Byte     *DP;
    DWord     DS;
    TInt64Rec V;
    String    S, SV;

    OutS = "";
    if (!Value.ValPtr) {
        V.Hi = Value.ValSz;
        V.Lo = Value.Val;
        DP   = reinterpret_cast<Byte *>(&V);
        DS   = 8;
    } else {
        DP = Value.ValPtr;
        DS = Value.ValSz;
    }
    bool MemVal = (Value.ValPtr != NULL);
    if (ShowGlobalTypeValue(Value.hDT, DP, DS, MemVal, Value.Kind, false, S) < 0 && !MemVal) {
        S     = ShowTypeName(Value.hDT);
        NDXHi = V.Hi;
        SV    = NDXToStr(V.Lo);
        OutLog2("%s", SV.c_str());
        S += SV;
    }
    OutS += S;
}
//------------------------------------------------------------------------------
void TConstDeclBase::Visit(TDCURecVisitor *Visitor) { Visitor->visitConstDeclBase(this); }
//------------------------------------------------------------------------------
void __fastcall TConstDeclBase::Show(String &OutS) {
    // TInlineDeclModifier *InlineCode;
    String S;

    PCONSTINFO constInfo = new CONSTINFO;
    constInfo->ModuleID  = ModuleID;
    constInfo->Type      = CI_CONSTDECL;
    constInfo->TypeDef   = "";
    constInfo->RTTIOfs   = 0xFFFFFFFF;
    constInfo->RTTISz    = 0;
    constInfo->Fixups    = new TList;

    OutS = "";
    TNameFDecl::Show(S); // inherited Show;
    constInfo->Name = S;

    ShowTypeName(Value.hDT);

    OutS += S + "=";
    OutLog1("=");

    pDumpOffset = &constInfo->RTTIOfs;
    pDumpSize   = &constInfo->RTTISz;
    FixupsList  = constInfo->Fixups;

    ShowValue(S);

    pDumpOffset = 0;
    pDumpSize   = 0;
    FixupsList  = 0;

    constInfo->Value = S;
    OutS += S;

    // todo?
    // if (FVer > verD4 && Value.Kind != 0)
    //     PutSFmtRemAux('Kind:#%x',[Value.Kind]);
    // InlineCode := TInlineDeclModifier(GetModifierOfClass(TInlineDeclModifier));
    // if (InlineCode) InlineCode.ShowInline(false);
    // Value.Show(!Name->IsEmpty || Def->Name->EqS('.'));

    if (ConstList)
        ConstList->Add(static_cast<void *>(constInfo));
    else {
        delete constInfo->Fixups;
        delete constInfo;
    }
}
//------------------------------------------------------------------------------
void __fastcall TConstDeclBase::EnumUsedTypes(TTypeUseAction Action, DWord *IP) {
    Action(this, Value.hDT, IP);
}
//------------------------------------------------------------------------------
Byte __fastcall TConstDeclBase::GetSecKind() {
    return skConst;
}
//------------------------------------------------------------------------------
TConstDecl::TConstDecl() : TConstDeclBase() {
    Value.hDT = ReadUIndex();
    Value.Read();

    /*hDT = ReadUIndex();
    if (FVer > verD4) {
        Kind = ReadUIndex();
        if (Kind < 0 || Kind > 5 || (Kind == 5 && !(FVer >= verD2009 && FVer < verK1)))
            printf("[Errorr] TConstDecl: Unknown const kind: #%d\n", Kind);
    }
    ReadConstVal(); // Value.Read;*/
}
//------------------------------------------------------------------------------
bool __fastcall TConstDecl::IsVisible(Byte LK) {
    if (!Inf && (FVer <= verD4 || Value.Kind == 1) && Value.ValPtr && Value.ValSz > 8 && (int) (*Value.ValPtr) == -1) {
        PName NP = GetName();
        // The resource string value looks like this - it should be ignored
        if (NP && NP->Get1stChar() == '.') return false;
    }
    return !Adopted && TNameFDecl::IsVisible(LK);
}
//------------------------------------------------------------------------------
void TConstDecl::Visit(TDCURecVisitor *Visitor) { Visitor->visitConstDecl(this); }
//------------------------------------------------------------------------------
TResStrDef::TResStrDef() : TVarCDecl(false) {
    OfsR = Ofs;
    Ofs  = -1;
}
//------------------------------------------------------------------------------
void __fastcall TResStrDef::Show(String &OutS) {
    String S;

    PRESSTRINFO rsInfo = new RESSTRINFO;
    rsInfo->ModuleID = ModuleID;
    rsInfo->DumpOfs = 0xFFFFFFFF;
    rsInfo->DumpSz = 0;

    TNameFDecl::Show(S);
    rsInfo->Name = S;
    OutLog1(":");
    rsInfo->TypeDef = ShowTypeDef(hDT, NULL);
    rsInfo->Context = "";

    ShowGlobalConstValue(hDecl + 1, OutS);
    ResStrList->Add(static_cast<void *>(rsInfo));
}
//------------------------------------------------------------------------------
Byte __fastcall TResStrDef::GetSecKind() {
    return skResStr;
}
//------------------------------------------------------------------------------
TSetDeftInfo::TSetDeftInfo() : TDCURec() {
    hConst = ReadUIndex();
    hArg = ReadUIndex();

    TDCURec *DR = GetAddrDef(hArg);

    if (TLocalValDecl *lvd = dynamic_cast<TLocalValDecl *>(DR)) {
        lvd->hDeftVal = hConst;
        Adopted = true;

        DR = GetAddrDef(hConst);

        if (TConstDecl *cd = dynamic_cast<TConstDecl *>(DR)) {
            cd->Adopted = true;
        }
    }
}
//------------------------------------------------------------------------------
void TSetDeftInfo::Visit(TDCURecVisitor *Visitor) { Visitor->visitSetDeftInfo(this); }
//------------------------------------------------------------------------------
void __fastcall TSetDeftInfo::Show(String &OutS) {
    OutLog2("Let %s := ", GetAddrStr(hArg, false).c_str());
    ShowGlobalConstValue(hConst, OutS);
    OutLog1("\n");
}
//------------------------------------------------------------------------------
bool TSetDeftInfo::IsVisible(Byte LK) { return TDCURec::IsVisible(LK); }
//------------------------------------------------------------------------------
/**
 * These kind of records were observed in DRIntf.dcu of D2006 where the
 * unit has several records of the same structure:
 *   TID         = record Reserved: array[$1..$6] of Byte; end;
 *   TDatabaseID = record Reserved: array[$1..$6] of Byte; end;
 *   TTableID    = --//--
 *   TFieldID    = --//--
 *   TAttrID     = --//--
 * Now they use drCopyDecl to point to the 1st Reserved declaration instead of duplicating it
 */
TCopyDecl::TCopyDecl() : TNameDecl(false) {
    // inherited Create00; => TNameDecl(false);

    if (FVer < verD11) {
        hDecl = AppendAddrDef(this); // It looks like this tag always adds to the end of the address table and ignores hNextAddr
    }

    hBase = ReadUIndex(); // index of the address to copy from
    TDCURec *SrcDef = GetAddrDef(hBase);
    if (!SrcDef)
        printf("[Error] CopyDecl index #%lX not found\n", hBase); // DCUErrorFmt

    if (!SrcDef->InheritsFrom(__classid(TNameDecl)))
        // printf("[Error] CopyDecl index #%lX(%s) is not a TNameDecl\n", hBase, SrcDef->Name->GetStr().c_str()); // DCUErrorFmt
        printf("[Error] CopyDecl index #%lX(%s) is not a TNameDecl\n", hBase);

    Base = static_cast<TNameDecl *>(SrcDef);
    Def = Base->Def;
}
//------------------------------------------------------------------------------
void TCopyDecl::Visit(TDCURecVisitor *Visitor) { Visitor->visitCopyDecl(this); }
//------------------------------------------------------------------------------
void __fastcall TCopyDecl::Show(String &OutS) { Base->Show(OutS); }
//------------------------------------------------------------------------------
Byte __fastcall TCopyDecl::GetSecKind() { return Base->GetSecKind(); }
//------------------------------------------------------------------------------
// extern Byte *FMemPtr;

TProcDecl::TProcDecl(TDCURec *AnEmbedded, bool NoInf) : TMemBlockRef(NoInf) {
    Ofs         = -1;
    Embedded    = AnEmbedded;
    bool NoName = IsUnnamed();
    int DataF;

    switch (FVer) {
        case verD6: DataF = 0x8000; break; // The flag 1st appears here
        case verD7: DataF = 0x800;  break; // Then it changes
        default:    DataF = 0;      break; // And then the StrConstDecl had been introduced
    }

    JustData    = (F & DataF) != 0;
    MethodKind  = mkProc;
    Locals      = nullptr;
    B0          = ReadUIndex();
    Sz          = ReadUIndex();

    if (FVer >= verDXE1 && FVer < verK1)
        int X = ReadByte(); // ReadUIndex() - it was detected in verDXE2 and Ok for verDXE

    if (!NoName) {
        if (FVer > verD2) VProc = ReadUIndex();
        hDTRes = ReadUIndex();
        // if (FVer >= verDXE1 && FVer < verK1 && VProc == 0x4F && ((F1 & 0x40) != 0)) return;
        if (FVer > verD7 && FVer < verK1) hClass = ReadUIndex();
        Tag      = ReadTag();
        CallKind = ReadCallKind();

        // try {
        if (FVer >= verD2009 && FVer < verK1) {
            // Read template parameters
            if (Tag == drA5Info) Tag = ReadTag(); // always precedes drA6Info
            if (Tag == drA6Info) {
                FTemplateArgs = new TA6Def;
                Tag           = ReadTag();
            }
        }

        ReadDeclList(dlArgs, this, &Args);
        // } catch {
        // todo: catch exception
        // E.Message := SysUtils.Format('%s in proc %s',[E.Message,Name^.GetStr]);
        // }

        if (Tag != drStop1) printf("[Error] TProcDecl: Stop Tag\n"); // TagError

        PDCURec *ArgP = &Args;

        while (*ArgP) {
            TDCURec *Loc = *ArgP;
            TDCURecTag tg  = Loc->GetTag();
            if (tg != arVal && tg != arVar) break;
            ArgP = &Loc->Next;
        }
        Locals = *ArgP;
        *ArgP  = nullptr;
    }
}
//------------------------------------------------------------------------------
TProcDecl::~TProcDecl() {
    FreeDCURecList(Locals);
    FreeDCURecList(Args);
    FreeDCURecList(Embedded);
}
//------------------------------------------------------------------------------
// In Kylix are used the names of the kind '.<X>.'
// In Delphi 6 were noticed only names '..'
// In Delphi 9 were noticed names of the kind '.<X>'
// In Delphi XE3 were noticed names of the kind '$thunk_'
bool __fastcall TProcDecl::IsUnnamed() {
    if (Def->Name.IsEmpty() || Def->Name.EqS(".")) return true;
    // todo: verify
    // was:
    // if (FVer >= verD6 && FVer < verK1 && (Def->Name.Len == 2 && Def->Name.Name[0] == '.' && Def->Name.Name[1] == '.')) return true;
    if (FVer >= verD6 && FVer < verK1 && Def->Name.EqS("..")) return true;
    if (FVer >= verK1 || FVer >= verD8) {
        AnsiChar ch = Def->Name.Get1stChar();
        if (ch == '.') return true;
        if (ch == '$' && FVer >= verDXE1 && FVer < verK1) return true;
    }
    return false;
    /*    bool Result = (Def->Name.Len == 0) || (Def->Name.Len == 1 && Def->Name.Name[0] == '.')
        || ((FVer >= verD6) && (FVer < verK1) && (Def->Name.Len == 2 && Def->Name.Name[0] == '.' && Def->Name.Name[1] == '.'))
        || (((FVer >= verK1) || (FVer >= verD8)) && (Def->Name.Name[0] == '.'));
    return Result;*/
}
//------------------------------------------------------------------------------
DWord __fastcall TProcDecl::SetMem(DWord MOfs, DWord MSz) {
    if (Ofs != -1)
        printf("[Error] Trying to change procedure %s memory to $%x[$%x]", Name->GetStr().c_str(), MOfs, MSz); // DCUErrorFmt

    if (FromPackage) {
        Sz = MSz; // MSz means something else here
    } else if (Sz > MSz) {
        printf("[Error] Procedure %s: memory size mismatch (.[$%x]>$%x[$%x])", Name->GetStr().c_str(), Sz, MOfs, MSz);
    }

    Ofs = MOfs;
    return MSz - Sz; // it can happen for ($L file) with several procedures
}
//------------------------------------------------------------------------------
Byte __fastcall TProcDecl::GetSecKind() {
    return skProc;
}
//------------------------------------------------------------------------------
String CallKindName[5] = {"register", "cdecl", "pascal", "stdcall", "safecall"};

void __fastcall TProcDecl::ShowArgs(String &OutS, PPROCDECLINFO pInfo) {
    String S = "";
    OutS = "";

    if (FTemplateArgs) {
        OutS += "<";
        FTemplateArgs->Show(S);
        OutS += S;
        OutS += ">";
    }

    bool NoName = IsUnnamed();

    TDCURec *ArgL = Args;

    if (ArgL) {
        OutS += "(";
        OutLog1("(");
    }
    ShowDeclList(dlArgs, ArgL, S);
    OutS += S;
    if (ArgL) {
        OutS += ")";
        OutLog1(")");
    }
    if (!IsProc()) {
        OutS += ":";
        OutLog1(":");
        S = ShowTypeDef(hDTRes, nullptr);
        if (pInfo) pInfo->TypeDef = S;
        OutS += S;
    }
    if (CallKind != pcRegister) {
        OutS += ";";
        OutLog1(";");
        S = CallKindName[CallKind];
        OutS += S;
        OutLog2("%s", AnsiString(S).c_str());
    }
    if (FVer > verD3) {
        if (FVer < verD2005) {
            if ((VProc & 0x1000) != 0) {
                OutS += ";overload";
                OutLog1(";overload");
            }
        } else {
            if ((VProc & 0x800) != 0) {
                OutS += ";overload";
                OutLog1(";overload");
            }
            if ((VProc & 0x2000000) != 0) {
                OutS += ";inline";
                OutLog1(";inline");
            }
        }
    }
}
//------------------------------------------------------------------------------
void TProcDecl::AddLocal(TDCURec *Loc) {
    // Loc.Add(Locals);
    // LocalsList->Add(static_cast<void *>(info));
    LocalsList->Add(Locals);
}
//------------------------------------------------------------------------------
bool __fastcall TProcDecl::IsProc() { return TypeIsVoid(hDTRes); }
//------------------------------------------------------------------------------
void __fastcall TProcDecl::Visit(TDCURecVisitor *Visitor) { Visitor->visitProcDecl(this); }
//------------------------------------------------------------------------------
void __fastcall TProcDecl::ShowDef(bool All, String &OutS) {
    // DCU32: ShowProc(TShowProcCtx(Ord(All)));

    String S = "";

    PPROCDECLINFO pInfo = new PROCDECLINFO;
    pInfo->ModuleID     = ModuleID;
    pInfo->Embedded     = false;
    pInfo->CallKind     = CallKind;
    pInfo->DumpOfs      = 0xFFFFFFFF;
    pInfo->DumpSz       = 0;
    pInfo->VProc        = VProc;

    if (IsProc()) {
        switch (MethodKind) {
            case mkConstructor:
                pInfo->MethodKind = 'C';
                OutLog1("constructor ");
                break;
            case mkDestructor:
                pInfo->MethodKind = 'D';
                OutLog1("destructor ");
                break;
            default:
                pInfo->MethodKind = 'P';
                OutLog1("procedure ");
                break;
        }
    } else {
        pInfo->MethodKind = 'F';
        OutLog1("function ");
    }
    TNameFDecl::Show(S);
    pInfo->Name = S;

    if (Def->Name.IsEmpty()) OutLog1("?");

    pInfo->Args = new TList;
    ArgsList    = pInfo->Args;
    ShowArgs(S, pInfo);
    ArgsList = 0;
    if (All) {
        pInfo->Locals = new TList;
        pInfo->Fixups = new TList;

        OutLog1(";\n");
        if (Locals) {
            LocalsList = pInfo->Locals;
            ShowDeclList(dlEmbedded, Locals, S);
            LocalsList = 0;
        }
        if (Embedded) {
            pInfo->Embedded = true;
            ShowDeclList(dlEmbedded, Embedded, S);
        }
        OutLog1("begin ");
        // GetRegVarInfo = GetRegDebugInfo;

        pDumpOffset = &pInfo->DumpOfs;
        pDumpSize   = &pInfo->DumpSz;
        FixupsList  = pInfo->Fixups;

        if (!IsUnnamed()) {
            pInfo->DumpType = 'C';
            OutLog1("code\n");
            ShowCodeBl(AddrBase, Ofs, Sz);
        } else {
            pInfo->DumpType = 'D';
            OutLog1("data\n");
            ShowDataBl(AddrBase, Ofs, Sz);
        }
        pDumpOffset = 0;
        pDumpSize   = 0;
        FixupsList  = 0;

        // GetRegVarInfo = NULL;
        ProcList->Add(static_cast<void *>(pInfo));
        OutLog1("end");
    } else {
        delete pInfo->Args;
        delete pInfo;
    }
    OutLog1("\n");
}
//------------------------------------------------------------------------------
void __fastcall TProcDecl::Show(String &OutS) {
    // ShowProc(spcOther);
    ShowDef(true, OutS);
}
//------------------------------------------------------------------------------
void __fastcall TProcDecl::EnumUsedTypes(TTypeUseAction Action, DWord *IP) {
    EnumUsedTypeList(Args, Action, IP);
    if (!IsProc()) Action(this, hDTRes, IP);
}
//------------------------------------------------------------------------------
bool __fastcall TProcDecl::IsVisible(Byte LK) {
    if (LK == dlMain) return (((F & 0x40) != 0) && (MethodKind == mkProc) && (hClass == 0));
    return true;
}
//------------------------------------------------------------------------------
void __fastcall TProcDecl::MemRefFound() {
    if (IsUnnamed()) JustData = true; // Mark the procedure as having no code
}
//------------------------------------------------------------------------------
bool TProcDecl::IsStaticMethod() {
    TDCURec *ArgL = Args;
    return FVer >= verDXE7 && MethodKind != mkProc && (!ArgL || !ArgL->Name->EqS("Self"));
}
//------------------------------------------------------------------------------
TSysProcDecl::TSysProcDecl() : TNameDecl(true) {
    F = ReadUIndex();
    Ndx = ReadIndex();
}
//------------------------------------------------------------------------------
void TSysProcDecl::Visit(TDCURecVisitor *Visitor) { Visitor->visitSysProcDecl(this); }
//------------------------------------------------------------------------------
void __fastcall TSysProcDecl::Show(String &OutS) {
    OutLog1("sysproc ");
    TNameDecl::Show(OutS);
}
//------------------------------------------------------------------------------
Byte __fastcall TSysProcDecl::GetSecKind() { return skProc; }
//------------------------------------------------------------------------------
TSysProc8Decl::TSysProc8Decl() : TProcDecl(NULL, true) {}
//------------------------------------------------------------------------------
void TSysProc8Decl::Visit(TDCURecVisitor *Visitor) { Visitor->visitSysProc8Decl(this); }
//------------------------------------------------------------------------------
TUnitAddInfo::TUnitAddInfo() : TNameFDecl(false) {
    B = ReadUIndex();
    Tag = ReadTag();
    ReadDeclList(dlUnitAddInfo, nullptr, &Sub);
}
//------------------------------------------------------------------------------
TUnitAddInfo::~TUnitAddInfo() {
    FreeDCURecList((TDCURec*)Sub);
}
//------------------------------------------------------------------------------
bool __fastcall TUnitAddInfo::IsVisible(Byte LK) { return false; }
//------------------------------------------------------------------------------
void TUnitAddInfo::Visit(TDCURecVisitor *Visitor) { Visitor->visitUnitAddInfo(this); }
//------------------------------------------------------------------------------
TSpecVar::TSpecVar() : TVarDecl() {}
//------------------------------------------------------------------------------
void TSpecVar::Visit(TDCURecVisitor *Visitor) { Visitor->visitSpecVar(this); }
//------------------------------------------------------------------------------
void __fastcall TSpecVar::Show(String &OutS) {
    String  S;

    OutLog1("spec var ");

    PVARINFO vInfo = new VARINFO;
    vInfo->ModuleID = ModuleID;
    vInfo->Type = VI_SPECVAR;
    vInfo->DumpOfs = 0xFFFFFFFF;
    vInfo->DumpSz = 0;
    vInfo->AbsName = "";

    TNameFDecl::Show(S);
    vInfo->Name = S;
    OutS = S + ":";
    OutLog1(":");
    pDumpOffset = &vInfo->DumpOfs;
    pDumpSize = &vInfo->DumpSz;
    S = ShowTypeDef(hDT, nullptr);
    pDumpOffset = 0;
    pDumpSize = 0;

    vInfo->TypeDef = S;
    OutS += S;
    VarList->Add(static_cast<void *>(vInfo));
}
//------------------------------------------------------------------------------
TTypeDef::TTypeDef() : TBaseDef(nullptr, reinterpret_cast<PDef>(DefStart), -1) {
    RTTISz   = ReadUIndex();
    Sz       = ReadIndex();
    hAddrDef = ReadUIndex();

    if (IsMSIL) {
        ReadUIndex();
        ReadUIndex();
    } else if (FVer >= verD2005 && FVer < verK1) {
        X = ReadUIndex();
    }

    FhDT = AddTypeDef(this);
    RTTIOfs = -1;
}
//------------------------------------------------------------------------------
TTypeDef::~TTypeDef() {
    ClearLastTypeDef(this);
    delete FModifiers;
}
//------------------------------------------------------------------------------
void TTypeDef::Visit(TDCURecVisitor *Visitor) { Visitor->visitTypeDef(this); }
//------------------------------------------------------------------------------
void __fastcall TTypeDef::ShowBase() {
    if (RTTISz > 0) ShowDataBl(0, RTTIOfs, RTTISz);
}
//------------------------------------------------------------------------------
int __fastcall TTypeDef::ShowValue(Byte* DP, DWord DS, String& OutS) {
    OutS = "";
    if (Sz > DS) return -1;
    // ShowDump(DP, NULL, 0, 0, Sz, 0, 0, 0, 0, NULL);
    ShowDataBlP(DP, Sz, 0);
    return Sz;
}
//------------------------------------------------------------------------------
void __fastcall TTypeDef::Show(String &OutS) {
    ShowBase();
    OutS = "";
}
//------------------------------------------------------------------------------
DWord __fastcall TTypeDef::SetMem(DWord MOfs, DWord MSz) {
    RTTIOfs = MOfs;
    return 0;
}
//------------------------------------------------------------------------------
String __fastcall TTypeDef::GetOfsQualifier(int Ofs) {
    if (Ofs == 0) return "";
    if (Ofs < Sz) return Sysutils::Format(".byte[%d]", ARRAYOFCONST((Ofs)));
    return Sysutils::Format(".?%d", ARRAYOFCONST((Ofs))); // Error
}
//------------------------------------------------------------------------------
String __fastcall TTypeDef::GetRefOfsQualifier(int Ofs) {
    if (Ofs == 0) return "^";
    return Sysutils::Format(".?%d", ARRAYOFCONST((Ofs))); // Error
}
//------------------------------------------------------------------------------
void TTypeDef::AddModifier(TDeclModifier *M) {
    TDeclModifier *MP = FModifiers;
    while (MP) {
        M = M->Next;
    }
    MP = M;
}
//------------------------------------------------------------------------------
TDeclModifier *TTypeDef::GetModifierOfClass(TDeclModifierClass *Cl) {
    return GetDeclModifierOfClass(FModifiers, Cl);
}
//------------------------------------------------------------------------------
TRangeBaseDef::TRangeBaseDef() : TTypeDef() {}
//------------------------------------------------------------------------------
void __fastcall TRangeBaseDef::GetRange(PInt64Rec Lo, PInt64Rec Hi) {
    Byte *Tmp = CurPos; // ChangeScanState(CP0,LH,18);
    CurPos = LH;
    ReadIndex64(Lo);
    ReadIndex64(Hi);
    CurPos = Tmp; // ChangeScanState(CP0,Enum.LH,18);
}
//------------------------------------------------------------------------------
void TRangeBaseDef::Visit(TDCURecVisitor *Visitor) { Visitor->visitRangeBaseDef(this); }
//------------------------------------------------------------------------------
String __fastcall WCharStr(wchar_t WCh) {
    wchar_t WStr[2];
    String S;
    char Ch, buf[256];

    if (static_cast<Word>(WCh) < 0x100)
        Ch = WCh;
    else {
        WStr[0] = WCh;
        WStr[1] = 0;
        S = WideCharToString(WStr);
        if (S.Length() > 0)
            Ch = S[1];
        else
            Ch = '.';
    }

    if (Ch < ' ')
        sprintf(buf, "#%d", static_cast<Word>(WCh));
    else
        sprintf(buf, "#$%lX", static_cast<Word>(WCh));

    return String(buf);
}
//------------------------------------------------------------------------------
String __fastcall BoolStr(char *DP, DWord DS) {
    char *CP = DP + DS - 1;
    while (CP > DP && *CP == 0) CP--;
    if ((CP = DP)) {
        if (*CP == 0) return "false";
        if (*CP == 1) return "true";
    }
    return "true";
}
//------------------------------------------------------------------------------
typedef struct {
    char Ch0;
    char Ch1;
} TByteChars;

char Digit[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

Word __fastcall ByteChars(Byte B) {
    TByteChars Ch;

    Word *Result = (Word *) &Ch;

    Ch.Ch0 = Digit[B >> 4];
    Ch.Ch1 = Digit[B & 0xF];
    return *Result;
}
//------------------------------------------------------------------------------
String __fastcall IntLStr(Byte* DP, DWord Sz, bool Neg) {
    char buf[256];

    if (Neg) {
        int  V;
        bool Ok = true;
        switch (Sz) {
            case 1: V = *reinterpret_cast<char *>(DP); break;
            case 2: V = *reinterpret_cast<short *>(DP); break;
            case 4: V = *reinterpret_cast<int *>(DP); break;
            default:
                Ok = false;
                if (Sz == 8) {
                    V = *reinterpret_cast<int *>(DP);
                    DP += 4;
                    NDXHi = *reinterpret_cast<int *>(DP);
                    return NDXToStr(V);
                }
                break;
        }
        if (Ok) {
            if (V >= 0)
                sprintf(buf, "$%lX", V);
            else
                sprintf(buf, "-$%lX", -V);
            return String(buf);
        }
    }

    Byte  *BP = DP + Sz - 1;
    String Result;
    Result.SetLength(Sz*2 + 1);
    char *P = AnsiString(Result).c_str();
    *P = '$';
    P++;
    for (int i = 1; i <= Sz; i++) {
        *reinterpret_cast<Word *>(P) = ByteChars(*BP);
        P += 2;
        BP--;
    }
    return Result;
}
//------------------------------------------------------------------------------
bool TRangeBaseDef::IsChar() {
    if (!Def) return false;
    // todo: verify
    TDCURecTag Tag = reinterpret_cast<TDCURecTag>(Def);
    // was: TDCURecTag Tag = Def->Tag;
    return Tag == drChRangeDef || Tag == drWCharRangeDef;
}
//------------------------------------------------------------------------------
/**
 *
 * @param DP
 * @param DS
 * @param OutS
 * @return Size used
 */
int __fastcall TRangeBaseDef::ShowValue(Byte* DP, DWord DS, String& OutS) {
    TDCURecTag Tag;
    OutS = "";

    if (Sz > DS) return -1;

    int Result = Sz; // unused

    if (!Def)
        Tag = drRangeDef;
    else
        Tag = reinterpret_cast<TDCURecTag>(Def); // Tag = Def->Tag; // todo: verify

    switch (Tag) {
        case drChRangeDef:
            if (Sz == 1) {
                OutS = CharStr(*reinterpret_cast<char *>(DP));
                OutLog2("%s", OutS.c_str());
                return 1;
            }
            break;
        case drWCharRangeDef:
            if (Sz == 2) {
                OutS = WCharStr(*reinterpret_cast<wchar_t *>(DP));
                OutLog2("%s", OutS.c_str());
                return 2;
            }
            break;
        case drBoolRangeDef:
            OutS = BoolStr(reinterpret_cast<char *>(DP), Sz);
            OutLog2("%s", OutS.c_str());
            return Sz;
    }
    Byte *Tmp = CurPos;
    CurPos = LH;
    TNDX Lo   = ReadIndex();
    bool Neg  = (NDXHi < 0);
    CurPos = Tmp;
    OutS = IntLStr(DP, Sz, Neg);
    OutLog2("%s", OutS.c_str());
    return Sz;
}
//------------------------------------------------------------------------------
void __fastcall TRangeBaseDef::Show(String &OutS) {
    TInt64Rec Lo, Hi;
    String Value, S;

    OutS = "";
    TTypeDef::Show(OutS);
    GetRange(&Lo, &Hi);
    TTypeDef *T = GetGlobalTypeDef(hDTBase);

    if (!T || ShowTypeValue(T, reinterpret_cast<Byte *>(&Lo), 8, 0, false, S) < 0) {
        NDXHi = Lo.Hi;
        Value = NDXToStr(Lo.Lo);
        S     = Value;
        OutLog2("%s", AnsiString(Value).c_str());
    }

    OutS += S;
    OutS += "..";
    OutLog1("..");

    if (!T || ShowTypeValue(T, reinterpret_cast<Byte *>(&Hi), 8, 0, false, S) < 0) {
        NDXHi = Hi.Hi;
        Value = NDXToStr(Hi.Lo);
        S     = Value;
        OutLog2("%s", AnsiString(Value).c_str());
    }
    OutS += S;
}
//------------------------------------------------------------------------------
void __fastcall TRangeBaseDef::EnumUsedTypes(TTypeUseAction Action, DWord *IP) {
    Action(this, hDTBase, IP);
}
//------------------------------------------------------------------------------
TRangeDef::TRangeDef() : TRangeBaseDef() {
    hDTBase = ReadUIndex();
    LH = CurPos;
    DWord Lo = ReadIndex();
    DWord Hi = ReadIndex();
    if (FVer >= verD8 && FVer < verK1)
        B = ReadUIndex();
    else
        B = ReadByte(); // It could be an index too, but I'm not sure
}
//------------------------------------------------------------------------------
void TRangeDef::Visit(TDCURecVisitor *Visitor) { Visitor->visitRangeDef(this); }
//------------------------------------------------------------------------------
TEnumDef::TEnumDef() : TRangeBaseDef() {
    hDTBase = ReadUIndex();
    if (FVer >= verD2009 && FVer < verK1) ReadUIndex();
    Ndx = ReadIndex();
    LH = CurPos;
    TNDX Lo = ReadIndex();
    TNDX Hi = ReadIndex();
    if (FVer >= verD8 && FVer < verK1)
        B = ReadUIndex();
    else
        B = ReadByte(); // It could be index too, but I'm not sure
}
//------------------------------------------------------------------------------
TEnumDef::~TEnumDef() {
    if (NameTbl) {
        if (NameTbl->Count > 0) FreeDCURecList((TDCURec *) (NameTbl->Items[0]));
        delete NameTbl;
    }
}
//------------------------------------------------------------------------------
int __fastcall TEnumDef::ShowValue(Byte *DP, DWord DS, String &OutS) {
    DWord  V;
    DWord  V0;
    String S;
    TConstDecl *C = nullptr;

    OutS = "";
    if (Sz > DS) return -1;
    if (!MemToUInt(DP, Sz, &V)) {
        if (!NameTbl || V >= NameTbl->Count) { // || V < 0
            V0 = static_cast<TConstDecl *>(NameTbl->Items[0])->Value.Val;
            V--;
            V0--;
            if (V >= 0 && V < NameTbl->Count) {
                C = static_cast<TConstDecl *>(NameTbl->Items[V]);
            }
        } else {
            C = CStart;
            while (!C) {
                if (C->Value.Val == V) {
                    break;
                }
                C = static_cast<TConstDecl *>(C->Next);
            }
        }
    }

    if (!C) {
        ShowName(S);
        OutS += S + "(";
        OutLog1("(");
        TRangeBaseDef::ShowValue(DP, DS, S);
        OutS += S + ")";
        OutLog1(")");
    }
    // ((TConstDecl *) (NameTbl->Items[V]))->ShowName(OutS);
    C->ShowName(OutS);
    return Sz;
}
//------------------------------------------------------------------------------
void __fastcall TEnumDef::Show(String &OutS) {
    AnsiString Name;

    if (!CStart) {
        TRangeBaseDef::Show(OutS);
        return;
    }

    ShowBase();
    OutS = "(";
    OutLog1("(");

    TConstDecl *EnumConst = CStart;

    int V = 0;
    int i = 0;

    while (!EnumConst) {
        if (i > 0) {
            OutS += ",";
            OutLog1(",");
        }
        Name = EnumConst->Name->GetStr();
        OutS += Name;
        OutLog2("%s", Name.c_str());

        if (EnumConst->Value.Val != V) {
            OutS += "=";
            OutLog1("=");
            V = EnumConst->Value.Val;
            OutS += V;
            OutLog2("%d", V);
        }
        V++;
        i++;

        EnumConst = static_cast<TConstDecl *>(EnumConst->Next);
    }
    OutS += ")";
    OutLog1(");");

    // was:
    /*for (int i = 0; i < NameTbl->Count; i++) {
        if (i > 0) {
            OutS += ",";
            OutLog1(",");
        }
        EnumConst = (TConstDecl *) NameTbl->Items[i];
        Name      = EnumConst->Name->GetStr();
        OutS += Name;
        OutLog2("%s", Name.c_str());
    }
    OutS += ")";
    OutLog1(");");*/
}
//------------------------------------------------------------------------------
void TEnumDef::Visit(TDCURecVisitor *Visitor) { Visitor->visitEnumDef(this); }
//------------------------------------------------------------------------------
// todo: review
TFloatDef::TFloatDef() : TTypeDef() {

    Byte B = ReadByte();

    const std::uint32_t FloatSz[fkCurrency + 1] = {
        10, // sizeof(System::Real) // todo: this depends, can be 6 or 10
        sizeof(Single),
        sizeof(Double),
        10, // sizeof(Extended)
        sizeof(Comp),
        sizeof(Currency)
    };

    // typedef long double          Extended;         // 10 byte real
    // typedef float                Single;           // 4 byte real
    // typedef Single*              PSingle;          //
    // typedef double               Double;           // 8 byte real

    if (FVer >= verDXE3) {
        if ((B & fkExtra) != 0) {
            B = B & ~fkExtra;
            ReadByte();
        }
    }

    if (B > fkCurrency) {
        printf("Unknown float kind: %d\n", B);
        // DCUErrorFmt('Unknown float kind: %d',[B]);
        return;
    }

    Kind   = B; // TFloatKind
    DWord KindSz = FloatSz[Kind];

    if (Kind == fkExtended) {
        switch (FPlatform) {
            case dcuplOsx32:
            case dcuplOsx64:
            case dcuplLinux64:
                if (FVer >= verD10_1) {
                    KindSz = 16;
                }
                break;
        }
    }
    if (KindSz != Sz)
        printf("[Error] Float kind and size mismatch: SizeOf(%s)=%d\n", GetKindName().c_str(), Sz); // DCUErrorFmt

}
//------------------------------------------------------------------------------
AnsiString __fastcall TFloatDef::GetKindName() { // was String
    // return GetEnumName(TypeInfo(TFloatKind), Ord(Kind));
    switch (Kind) {
        case 0: return "fkReal48";
        case 1: return "fkSingle";
        case 2: return "fkDouble";
        case 3: return "fkExtended";
        case 4: return "fkComp";
        case 5: return "fkCurrency";
    }
}
//------------------------------------------------------------------------------
void TFloatDef::Visit(TDCURecVisitor *Visitor) { Visitor->visitFloatDef(this); }
//------------------------------------------------------------------------------
int __fastcall TFloatDef::ShowValue(Byte *DP, DWord DS, String &OutS) {
    long double E;
    PName       N;

    OutS = "";
    if (Sz > DS) return -1;
    bool Ok = true;
    switch (DS) {
        case 4: // SizeOf(Single)
            E = *reinterpret_cast<float *>(DP);
            break;
        case 8: // SizeOf(Double)
            N = Name;
            if (!N)
                Ok = false;
            else {
                if (!CompareText(N->GetStr(), "Double")) {
                    E = *reinterpret_cast<double *>(DP);
                } else if (!CompareText(N->GetStr(), "Currency")) {
                    // if (FVer <= verD4 && DS == 10) {
                        Int64 rawCurrency = *reinterpret_cast<Int64 *>(DP);
                        E = static_cast<long double>(rawCurrency) / 10000.0L;
                        // or: E = *reinterpret_cast<Extended *>(DP) * 0.0001;
                    // } else {}
                } else if (!CompareText(N->GetStr(), "Comp"))
                    E = *reinterpret_cast<Comp *>(DP);
                else {
                    Ok = false;
                }
            }
            break;
        case 10: // SizeOf(Extended)
            E = *reinterpret_cast<long double *>(DP);
            break;
        case 16: // Extended128 of Linux 64 and OSX 64
            // todo
            DP += 6; // The Extended128 just adds 6 bytes to the mantissa
                     // in comparison with Extended80, the rest is the same, so we just skip these bytes here
            // E = *reinterpret_cast<TExtended80Rec *>(DP);
            break;
        default:
            Ok = false;
            break;
    }
    if (Ok) {
        OutS = Sysutils::Format("%g", ARRAYOFCONST((E)));
        OutLog2("%s", OutS.c_str());
        return Sz;
    }
    return TTypeDef::ShowValue(DP, Sz, OutS);
}
//------------------------------------------------------------------------------
void __fastcall TFloatDef::Show(String &OutS) {
    String S;
    OutS = "float(" + GetKindName() + ")";
    OutLog2("float(%s)", OutS.c_str());
    TTypeDef::Show(S);
    OutS += S;
}
//------------------------------------------------------------------------------
TPtrDef::TPtrDef() : TTypeDef() {
    hRefDT = ReadUIndex();
    if (FVer >= verD2009 && FVer < verK1) ReadUIndex();
}
//------------------------------------------------------------------------------
// From DCU_In.pas:
// Set FixUpEnd to the max(FixUpEnd,CodeFixups^.Ofs+4 if CodeFixups^.F is not fxStart or fxEnd
void __fastcall SetFixEnd() {
    DWord CurOfs;
    CurOfs = CodeFixups->OfsF;
    Byte F = reinterpret_cast<Byte *>(&CurOfs)[3];
    CurOfs = CurOfs & FixOfsMask;
    if (fxValid.Contains(F)) {
        // if (F < fxStart) {
        int Sz = fxSize[F];
        if (Sz >= 0) {
            Byte *EP = CodeStart + CurOfs + Sz; // 4
            if (EP > FixUpEnd) FixUpEnd = EP;
        }
    }
}
//------------------------------------------------------------------------------
// Move CodeFixups to the next fixup with Offset>=Ofs
void __fastcall SkipFixups(DWord Ofs) {
    while (CodeFixupCnt > 0) {
        if ((CodeFixups->OfsF & FixOfsMask) >= Ofs) break;
        SetFixEnd();
        CodeFixups++;
        CodeFixupCnt--;
    }
}
//------------------------------------------------------------------------------
// If CodeFixups^ has the Offset=Ofs return it, else - Nil
PFixupRec __fastcall CurFixup(DWord Ofs) {
    if (CodeFixupCnt > 0 && ((CodeFixups->OfsF & FixOfsMask) == Ofs))
        return CodeFixups;
    return nullptr;
}
//------------------------------------------------------------------------------
// Move CodeFixups to the next fixup, Return true if the next fixup has the Offset<=Ofs
bool __fastcall NextFixup(DWord Ofs) {
    if (CodeFixupCnt <= 0) return false;
    SetFixEnd();
    CodeFixups++;
    CodeFixupCnt--;
    if (CodeFixupCnt <= 0) return false;
    if ((CodeFixups->OfsF & FixOfsMask) > Ofs) return false;
    return true;
}
//------------------------------------------------------------------------------
bool __fastcall GetFixupFor(Byte *CodePtr, DWord Size, bool StartOk, PFixupRec *Fix) {
    *Fix = nullptr;
    if (CodePtr + Size > CodeEnd) return false;
    DWord Ofs = CodePtr - CodeStart;
    if (Size == 4 || (fx8Byte && Size == 8)) { // All fixups are 4 bytes
        SkipFixups(Ofs);
        if (CodePtr < FixUpEnd) return false;
        do {
            PFixupRec Fx = CurFixup(Ofs);
            if (!Fx) break;
            Byte F = reinterpret_cast<Byte *>(&Fx->OfsF)[3];
            if (fxValid.Contains(F)) { // F < fxStart
                if (*Fix) return false;
                *Fix = Fx; // The difference between fxAddr and fxJmpAddr could also be taken into account
            } else if (F != fxStart || !StartOk)
                return false;
        } while (NextFixup(Ofs));
        FixUpEnd = CodePtr;
    }
    SkipFixups(Ofs + Size);
    if (CodePtr < FixUpEnd) return false;
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
    }
    return "";
}
//------------------------------------------------------------------------------
// This function should check whether DP points to some valid text
int __fastcall TryShowPCharConst(Byte *DP, DWord DS) {
    Byte *EP = DP + DS;
    Byte *CP = DP;
    while (CP < EP && (*CP == 9 || *CP == 0x13 || *CP == 0x10 || *CP >= 0x32)) CP++;
    if (CP >= EP || *CP) return -1;
    if (*DP == 0xE9 && CP == DP + 1) return -1;
    OutLog2("%s", StrConstStr(reinterpret_cast<char *>(DP), CP - DP).c_str());
    return CP - DP + 1;
}
//------------------------------------------------------------------------------
bool __fastcall ReportFixup(PFixupRec Fix, int Ofs, bool UseHAl) {
    DWord  Sz;
    String OutS;
    TDCURec *Member = nullptr;
    TBaseDef *TD = nullptr;

    if (!Fix) return false;

    Byte K = reinterpret_cast<Byte *>(&Fix->OfsF)[3];
    OutLog2("K%d ", K);

    TDCURec *D   = GetGlobalAddrDef(Fix->Ndx);
    int hDT = -1;
    int L   = -1;

    if (D) {
        if (D->InheritsFrom(__classid(TVarDecl)))
            hDT = static_cast<TVarDecl *>(D)->hDT;
        else if (UseHAl && Ofs > 0 && D->InheritsFrom(__classid(TProcDecl))) {
            Byte *DP = GetBlockMem(static_cast<TProcDecl *>(D)->Ofs, static_cast<TProcDecl *>(D)->Sz, &Sz);
            if (DP && Ofs <= Sz) {
                if (Ofs >= 8) L = ShowStrConst(DP + Ofs - 8, Sz - Ofs + 8, OutS);
                if (L < 0) L = TryShowPCharConst(DP + Ofs, Sz - Ofs);
            }
        }
    }

    if (L > 0) OutLog1(" {");

    // todo:
    // if TUnit(FixUnit).IsMSIL and(K=fxVirtMethodMSIL) then begin

    String S;

    if (IsMSIL && K == fxVirtMethodMSIL) {
        Member = nullptr;
        if (D) {
            if (TTypeDecl *D = dynamic_cast<TTypeDecl *>(D)) {
                hDT = D->hDef;

                TD = nullptr;

                if (hDT >= 0)
                    TD = GetTypeDef(hDT);

                if (TD) {
                    if (TRecBaseDef *TBD = dynamic_cast<TRecBaseDef *>(TD)) {
                        Member = static_cast<TRecBaseDef *>(TBD->GetMemberByNum(Ofs - 1));

                        if (Member)
                            Member->ShowName(S);
                    }
                }

            }
        }
    }

    OutLog2("%s", GetAddrStr(Fix->Ndx, false).c_str());
    OutLog2("%s", ShowOfsQualifier(hDT, Ofs).c_str());

    if (L > 0) OutLog1("}");

    return true;
}
//------------------------------------------------------------------------------
typedef bool __fastcall (__closure *TShowPtrValProc)(TNDX Ndx, DWord Ofs, String &OutS);

void __fastcall ShowPointer(Byte *DP, AnsiString NilStr, String &OutS, TShowPtrValProc ShowVal) {
    PFixupRec Fix;

    char HexVal[32];

    OutS = "";
    Byte *V = *reinterpret_cast<Byte **>(DP);

    if (GetFixupFor(DP, FPtrSize, true, &Fix) && Fix) {
        if (FPtrSize == 8) {
            // For 64-bit mode I suppose that the Hi part of offset should be 0 and use the same code as for 32-bit mode
            if (reinterpret_cast<PInt64Rec>(DP)->Hi != 0) {
                printf("[Error] Nonzero fixup offset high part: $%8.8x\n", reinterpret_cast<PInt64Rec>(DP)->Hi); // DCUErrorFmt
            }
        }
        PName FxName = GetAddrName(Fix->Ndx);
        bool vOK = (!FxName ||  FxName->IsEmpty()) && ShowVal && ShowVal(Fix->Ndx, reinterpret_cast<DWord>(V), OutS);

        OutS += "@";
        OutLog1("@");

        if (!ReportFixup(Fix, reinterpret_cast<DWord>(V), false)) {
            if (V) {
                sprintf(HexVal, "+$%lX", reinterpret_cast<DWord>(V));
                OutS += String(HexVal);
                OutLog2("%s", HexVal);
            }
        }
    } else if (FPtrSize == 8) {
        PInt64Rec p64 = reinterpret_cast<PInt64Rec>(DP);
        if (p64->Lo == 0 && p64->Hi == 0) {
            OutS += NilStr;
        } else {
            sprintf(HexVal, "%8.8x%8.8x", p64->Hi, p64->Lo);
            OutS += String(HexVal);
            OutLog2("%s", HexVal);
        }
    } else if (!V) {
        OutS += NilStr;
        OutLog2("%s", NilStr.c_str());
    } else {
        sprintf(HexVal, "$%.8lX", reinterpret_cast<DWord>(V));
        OutS += String(HexVal);
        OutLog2("%s", HexVal);
    }
}
//------------------------------------------------------------------------------
char *__fastcall StrLEnd(char *Str, DWord L) {
    if (L < strlen(Str)) return Str + L;
    return Str + strlen(Str) - 1;
}
//------------------------------------------------------------------------------
bool __fastcall TPtrDef::ShowRefValue(TNDX Ndx, DWord Ofs, String& OutS) {
    DWord Sz;
    // if (!FixUnit) return false; // TUnit->FixUnit
    TTypeDef *DT = GetGlobalTypeDef(hRefDT);
    // todo: verify:
    if (!DT || !DT->Def || reinterpret_cast<TDCURecTag>(DT->Def) != drChRangeDef) return false;
    TDCURec *AR = GetGlobalAddrDef(Ndx);
    if (!AR || !(AR->InheritsFrom(__classid(TMemBlockRef)))) return false;
    TMemBlockRef* memBlock = static_cast<TMemBlockRef*>(AR);
    char *DP = reinterpret_cast<char *>(GetBlockMem(memBlock->Ofs,memBlock->Sz, &Sz));
    if (Ofs >= Sz) return false;
    char *EP = StrLEnd(DP + Ofs, Sz - Ofs);
    if (EP - DP == Sz) return false;
    // We could also check that there are no fixups in the DP+Ofs..EP range
    OutS = StrConstStr(DP + Ofs, EP - (DP + Ofs));
    return true;
}
//------------------------------------------------------------------------------
void TPtrDef::Visit(TDCURecVisitor *Visitor) { Visitor->visitPtrDef(this); }
//------------------------------------------------------------------------------
int __fastcall TPtrDef::ShowValue(Byte *DP, DWord DS, String &OutS) {
    OutS = "";
    if (Sz > DS) return -1;
    if (Sz == FPtrSize) {
        ShowPointer(DP, "Nil", OutS, ShowRefValue);
        return Sz;
    }
    return TTypeDef::ShowValue(DP, Sz, OutS);
}
//------------------------------------------------------------------------------
void __fastcall TPtrDef::Show(String &OutS) {
    String S;
    OutS = "";
    TTypeDef::Show(S);
    OutS += S + "^";
    OutLog1("^");
    OutS += ShowTypeDef(hRefDT, NULL);
}
//------------------------------------------------------------------------------
void __fastcall TPtrDef::EnumUsedTypes(TTypeUseAction Action, DWord *IP) {
    Action(this, hRefDT, IP);
}
//------------------------------------------------------------------------------
String __fastcall TPtrDef::GetRefOfsQualifier(int Ofs) {
    return "^" + ShowOfsQualifier(hRefDT, Ofs);
}
//------------------------------------------------------------------------------
TTextDef::TTextDef() : TTypeDef() {}
//------------------------------------------------------------------------------
void TTextDef::Visit(TDCURecVisitor *Visitor) { Visitor->visitTextDef(this); }
//------------------------------------------------------------------------------
void __fastcall TTextDef::Show(String &OutS) {
    String S;
    TTypeDef::Show(S);
    OutS = "text";
    OutLog1("text");
}
//------------------------------------------------------------------------------
TFileDef::TFileDef() : TTypeDef() {
    hBaseDT = ReadUIndex();
}
//------------------------------------------------------------------------------
void TFileDef::Visit(TDCURecVisitor *Visitor) { Visitor->visitFileDef(this); }
//------------------------------------------------------------------------------
void __fastcall TFileDef::Show(String &OutS) {
    String S;
    TTypeDef::Show(S);
    OutS = "file of ";
    OutLog1("file of ");
    OutS += ShowTypeDef(hBaseDT, NULL);
}
//------------------------------------------------------------------------------
void __fastcall TFileDef::EnumUsedTypes(TTypeUseAction Action, DWord *IP) {
    Action(this, hBaseDT, IP);
}
//------------------------------------------------------------------------------
TSetDef::TSetDef() : TTypeDef() {
    BStart  = ReadByte();
    hBaseDT = ReadUIndex();
}
//------------------------------------------------------------------------------
void TSetDef::Visit(TDCURecVisitor *Visitor) { Visitor->visitSetDef(this); }
//------------------------------------------------------------------------------
int __fastcall TSetDef::ShowValue(Byte *DP, DWord DS, String &OutS) {
    int    K;
    Byte   B;
    String S;

    TInt64Rec V0, Lo, Hi;

    OutS = "";
    if (Sz > DS) return -1;
    TTypeDef *T = GetGlobalTypeDef(hBaseDT);

    if (!T || !T->InheritsFrom(__classid(TRangeBaseDef))) return -1;

    static_cast<TRangeBaseDef *>(T)->GetRange(&Lo, &Hi);
    Lo.Lo = BStart * 8;
    Hi.Lo = (BStart + Sz) * 8 - 1;
    OutS += "[";
    OutLog1("[");
    int  Cnt   = 0;
    bool SetOn = false;

    while (Lo.Lo <= Hi.Lo) {
        K = (Lo.Lo & 7);
        if (!K) {
            B = *DP;
            DP++;
        }
        bool WasOn = SetOn;
        SetOn = ((B & (1 << K)) != 0);
        if (WasOn != SetOn) {
            if (WasOn) {
                if (Cnt > 0) {
                    OutS += ",";
                    OutLog1(",");
                }
                Cnt++;
                ShowTypeValue(T, reinterpret_cast<Byte *>(&V0), sizeof(V0), 0, false, S);
                OutS += S;
                Lo.Lo--;
                if (V0.Lo != Lo.Lo) {
                    OutS += "..";
                    OutLog1("..");
                    ShowTypeValue(T, reinterpret_cast<Byte *>(&Lo), sizeof(Lo), 0, false, S);
                    OutS += S;
                }
                Lo.Lo++;
            } else
                V0.Lo = Lo.Lo;
        }
        Lo.Lo++;
    }
    if (SetOn) {
        if (Cnt > 0) {
            OutS += ",";
            OutLog1(",");
        }
        Cnt++;
        ShowTypeValue(T, reinterpret_cast<Byte *>(&V0), sizeof(V0), 0, false, S);
        OutS += S;
        Lo.Lo--;
        if (V0.Lo != Lo.Lo) {
            OutS += "..";
            OutLog1("..");
            ShowTypeValue(T, reinterpret_cast<Byte *>(&Lo), sizeof(Lo), 0, false, S);
            OutS += S;
        }
        Lo.Lo++;
    }
    OutS += "]";
    OutLog1("]");
    return Sz;
}
//------------------------------------------------------------------------------
void __fastcall TSetDef::Show(String &OutS) {
    String S;
    TTypeDef::Show(S);
    OutS = "set of ";
    OutLog1("set of ");
    OutS += ShowTypeDef(hBaseDT, NULL);
}
//------------------------------------------------------------------------------
void __fastcall TSetDef::EnumUsedTypes(TTypeUseAction Action, DWord *IP) {
    Action(this, hBaseDT, IP);
}
//------------------------------------------------------------------------------
// TArrayDef0
TArrayDef0::TArrayDef0(bool IsStr) : TTypeDef() {
    B1     = ReadByte();
    hDTNdx = ReadUIndex();
    hDTEl  = ReadUIndex();
    if (!IsStr && IsMSIL) ReadUIndex();
}
//------------------------------------------------------------------------------
void TArrayDef0::Visit(TDCURecVisitor *Visitor) { Visitor->visitArrayDef0(this); }
//------------------------------------------------------------------------------
int __fastcall TArrayDef0::ShowValue(Byte *DP, DWord DS, String &OutS) {
    String S;

    OutS = "";
    if (Sz > DS) return -1;
    TTypeDef *T = GetGlobalTypeDef(hDTEl);
    if (!T) return -1;
    if (T->Def && *reinterpret_cast<Byte *>(T->Def) == drChRangeDef) {
        OutS = StrConstStr(reinterpret_cast<char *>(DP), Sz);
        OutLog2("%s", OutS.c_str());
        return Sz;
    }
    DWord Rest = Sz;
    DWord ElSz = T->Sz;
    OutS       = "(";
    OutLog1("(");
    int Cnt = 0;
    while (Rest >= ElSz) {
        if (Cnt > 0) {
            OutS += ",";
            OutLog1(",");
        }
        if (ShowTypeValue(T, DP, Rest, -1, false, S) < 0) {
            return -1;
        }
        OutS += S;
        Cnt++;
        DP += ElSz;
        Rest -= ElSz;
    }
    OutS += ")";
    OutLog1(")");
    return Sz;
}
//------------------------------------------------------------------------------
void __fastcall TArrayDef0::Show(String &OutS) {
    OutS = "array";
    OutLog1("array");
    ShowBase();
    OutS += "[";
    OutLog1("[");
    OutS += ShowTypeDef(hDTNdx, nullptr);
    OutS += "] of ";
    OutLog1("] of ");
    OutS += ShowTypeDef(hDTEl, nullptr);
}
//------------------------------------------------------------------------------
void __fastcall TArrayDef0::EnumUsedTypes(TTypeUseAction Action, DWord *IP) {
    Action(this, hDTNdx, IP);
    Action(this, hDTEl, IP);
}
//------------------------------------------------------------------------------
// TArrayDef
TArrayDef::TArrayDef(bool IsStr) : TArrayDef0(IsStr) {}
//------------------------------------------------------------------------------
String __fastcall TArrayDef::GetOfsQualifier(int Ofs) {
    TTypeDef *TD = GetGlobalTypeDef(hDTEl);
    if (!TD || !Ofs)
        return TTypeDef::GetOfsQualifier(Ofs);
    else {
        int ElSz = TD->Sz;
        return Sysutils::Format("[%d]%s", ARRAYOFCONST((Ofs / ElSz, ShowOfsQualifier(hDTEl, Ofs % ElSz))));
    }
}
//------------------------------------------------------------------------------
void TArrayDef::Visit(TDCURecVisitor *Visitor) { Visitor->visitArrayDef(this); }
//------------------------------------------------------------------------------
// todo: review:
TTypeValKind TArrayDef::ValKind() {
    TTypeDef *TD = GetGlobalTypeDef(hDTEl);
    if (TRangeDef *RD = dynamic_cast<TRangeDef*>(TD)) {
        if (RD->IsChar()) return TTypeValKind::vkStr;
    }
    return TTypeValKind::vkComplex;
}
//------------------------------------------------------------------------------
TShortStrDef::TShortStrDef() : TArrayDef(true) {
    if (FVer >= verD2009 && FVer < verK1) CP = ReadUIndex();
}
//------------------------------------------------------------------------------
void TShortStrDef::Visit(TDCURecVisitor *Visitor) { Visitor->visitShortStrDef(this); }
//------------------------------------------------------------------------------
/**
 *
 * @param DP
 * @param DS
 * @param OutS
 * @return Size used
 */
int __fastcall TShortStrDef::ShowValue(Byte *DP, DWord DS, String &OutS) {
    OutS = "";
    if (Sz > DS) return -1;
    // was: int L = reinterpret_cast<PName>(DP)->Len;
    // todo: verify:
    int L = strlen(reinterpret_cast<const char *>(reinterpret_cast<PShortString>(DP)));
    if (L >= Sz)
        return TArrayDef::ShowValue(DP, DS, OutS);
    else {
        OutS = StrConstStr(reinterpret_cast<char *>(DP) + 1, L);
        OutLog2("%s", OutS.c_str());
        return Sz;
    }
}
//------------------------------------------------------------------------------
void __fastcall TShortStrDef::Show(String &OutS) {
    if (Sz == -1) {
        OutS = "ShortString";
        OutLog1("ShortString");
    } else {
        OutS = "String[" + String(Sz - 1) + "]";
        OutLog2("String[%d]", Sz - 1);
    }
    ShowBase();
}
//------------------------------------------------------------------------------
TStringDef::TStringDef() : TArrayDef(true) {
    if (FVer >= verD2009 && FVer < verK1) CP = ReadUIndex();
}
//------------------------------------------------------------------------------
void TStringDef::Visit(TDCURecVisitor *Visitor) { Visitor->visitStringDef(this); }
//------------------------------------------------------------------------------
bool __fastcall TStringDef::ShowRefValue(TNDX Ndx, DWord Ofs, String& OutS) {
    DWord Sz;
    int L;

    if (Ofs < 8) return false;
    TDCURec *AR = GetGlobalAddrDef(Ndx);
    if (!AR || !(AR->InheritsFrom(__classid(TMemBlockRef)))) return false;
    TMemBlockRef *Proc = static_cast<TMemBlockRef *>(AR);
    char *DP = reinterpret_cast<char *>(GetBlockMem(Proc->Ofs, Proc->Sz, &Sz));
    if (!DP || Ofs >= Sz) return false;
    Proc->MemRefFound();
    // if (Proc->IsUnnamed()) Proc->JustData = true; // Mark the procedure as having no code
    int ChSz = -1;
    if (FVer >= verD2009 && FVer < verK1) {
        ChSz = GetTypeSize(hDTEl);
    }
    if (ChSz == 2) {
        if (Ofs < 12) return false;
        L = ShowUnicodeStrConst(reinterpret_cast<Byte *>(DP + Ofs - 12), reinterpret_cast<DWord>(Sz - Ofs + 12), OutS);
    } else {
        if (Ofs < 8) return false;
        L = ShowStrConst(reinterpret_cast<Byte *>(DP + Ofs - 8), reinterpret_cast<DWord>(Sz - Ofs + 8), OutS);
    }
    return (L > 0);
}
//------------------------------------------------------------------------------
int __fastcall TStringDef::ShowValue(Byte* DP, DWord DS, String& OutS) {
    OutS = "";
    if (Sz > DS) return -1;
    if (Sz == FPtrSize) {
        ShowPointer(DP, "''", OutS, ShowRefValue);
        return Sz;
    }
    return TArrayDef::ShowValue(DP, Sz, OutS);
}
//------------------------------------------------------------------------------
void __fastcall TStringDef::Show(String &OutS) {
    OutS = "String";
    OutLog1("String");
    ShowBase();
}
//------------------------------------------------------------------------------
String __fastcall TStringDef::GetRefOfsQualifier(int Ofs) {
    TTypeDef *TD = GetGlobalTypeDef(hDTEl);
    if (!TD || !Ofs)
        return TTypeDef::GetRefOfsQualifier(Ofs);
    else {
        int ElSz = TD->Sz;
        return TTypeDef::GetOfsQualifier(Ofs + ElSz); // Because String is 1-based
    }
}
//------------------------------------------------------------------------------
TVariantDef::TVariantDef() : TTypeDef() {
    if (FVer > verD2) B = ReadByte();
}
//------------------------------------------------------------------------------
void TVariantDef::Visit(TDCURecVisitor *Visitor) { Visitor->visitVariantDef(this); }
//------------------------------------------------------------------------------
void __fastcall TVariantDef::Show(String &OutS) {
    OutLog1("variant");
    TTypeDef::Show(OutS);
    OutS = "variant";
}
//------------------------------------------------------------------------------
TObjVMTDef::TObjVMTDef() : TTypeDef() {
    hObjDT = ReadUIndex();
    Ndx1 = ReadUIndex();
    if (IsMSIL) ReadUIndex();
}
//------------------------------------------------------------------------------
void TObjVMTDef::Visit(TDCURecVisitor *Visitor) { Visitor->visitObjVMTDef(this); }
//------------------------------------------------------------------------------
void __fastcall TObjVMTDef::Show(String &OutS) {
    TTypeDef::Show(OutS);
    OutS += "class of ";
    OutLog1("class of ");
    OutS += ShowTypeDef(hObjDT, NULL);
}
//------------------------------------------------------------------------------
TRecBaseDef::TRecBaseDef() : TTypeDef() {}
//------------------------------------------------------------------------------
TRecBaseDef::~TRecBaseDef() { FreeDCURecList((TDCURec *) Fields); }
//------------------------------------------------------------------------------
void __fastcall TRecBaseDef::ReadFields(Byte LK) {
    Tag = ReadTag();
    ReadDeclList(LK, nullptr, &Fields);
    if (Tag != drStop1) printf("[Error] TRecBaseDef::ReadFields: Stop Tag\n"); // TagError
}
//------------------------------------------------------------------------------
void TRecBaseDef::Visit(TDCURecVisitor *Visitor) { Visitor->visitRecBaseDef(this); }
//------------------------------------------------------------------------------

/**
 * Attention: records with variants may be incorrectly shown (see readme.txt for details)}
 * @param DP
 * @param DS
 * @param OutS
 * @return Size used
 */
int __fastcall TRecBaseDef::ShowFieldValues(Byte *DP, DWord DS, String &OutS) {
    TDCURec *Decl;
    TDCURec *DeclL = Fields;
    String S;

    OutS = "";
    if (Sz > DS) return -1;
    int Cnt = 0;
    bool Ok = true;
    OutS += "(";
    OutLog1("(");

    while (DeclL) {
        Decl = DeclL;
        if (Decl->InheritsFrom(__classid(TCopyDecl))) Decl = static_cast<TCopyDecl *>(Decl)->Base;
        if (Decl->InheritsFrom(__classid(TLocalDecl)) && Decl->GetTag() == arFld) {
            if (Cnt > 0) {
                OutS += ";";
                OutLog1(";");
            }
            Decl->ShowName(S);
            OutS += S + ":";
            OutLog1(":");
            int Ofs = static_cast<TLocalDecl *>(Decl)->Ndx;
            if (Ofs < 0 || Ofs > Sz ||
                ShowGlobalTypeValue(static_cast<TLocalDecl *>(Decl)->hDT, DP + Ofs, Sz - Ofs, false, -1, false, S) < 0) {
                OutS += "?";
                OutLog1("?");
                Ok = false;
            }
            OutS += S;
            Cnt++;
        }
        DeclL = DeclL->Next;
        // DeclL = static_cast<TNameDecl *>(DeclL->Next);
    }
    OutS += ")";
    OutLog1(")");
    if (!Ok) {
        TTypeDef::ShowValue(DP, DS, S);
        OutS += S;
    }
    return Sz;
}
//------------------------------------------------------------------------------
void __fastcall TRecBaseDef::EnumUsedTypes(TTypeUseAction Action, DWord *IP) { EnumUsedTypeList(Fields, Action, IP); }
//------------------------------------------------------------------------------
TNDX __fastcall TRecBaseDef::GetParentType() { return -1; }
//------------------------------------------------------------------------------
// This procedure is required to find properties corresponding to unnamed fields
TPropDecl *__fastcall TRecBaseDef::GetFldProperty(PNameDecl Fld, int hDT) {
    TDCURec *Decl = Fld->Next; // It should go after the field

    while (Decl) {
        if (Decl->InheritsFrom(__classid(TPropDecl)) && static_cast<TPropDecl *>(Decl)->hDT == hDT) {
            TPropDecl *Result = static_cast<TPropDecl *>(Decl);
            if (Result->hRead && GetAddrDef(Result->hRead) == Fld) return Result;
            if (Result->hWrite && GetAddrDef(Result->hWrite) == Fld) return Result;
        }
        Decl = Decl->Next;
    }
    return nullptr;
}
//------------------------------------------------------------------------------
// For .Net fixups
TDCURec *GetDCURecListItemByNum(TDCURec *L, int Num) {
    if (Num < 0) return nullptr;
    while (Num > 0 && L) {
        Num--;
        L = L->Next;
    }
    return L;
}
//------------------------------------------------------------------------------
TDCURec *TRecBaseDef::GetMemberByNum(int Num) {
    return GetDCURecListItemByNum(Fields, Num);
}
//------------------------------------------------------------------------------
// todo?
// int __fastcall TRecBaseDef::GetFldOfsQualifier(int Ofs, int QSz, PQualInfo QI, int TotSize, bool Sorted, PAnsiString QS)
String __fastcall TRecBaseDef::GetFldOfsQualifier(int Ofs, int TotSize, bool Sorted) {
    if (Ofs >= TotSize) return ""; // -1

    // todo?
    // if (QS) *QS = '';

    TDCURec  *Decl;
    TTypeDef *FldTD;
    AnsiString FldName;
    TDCURec *DeclL = Fields;

    // todo?
    /*TLocalDecl *FldDecl = GetFldByOfs(Ofs,QSz,TotSize,Sorted);
    if (QS) {
        FldName = FldDecl->Name->GetStr();
        if (FldName == "") {
            TPropDecl *PropDecl = GetFldProperty(FldDecl,FldDecl.hDT);
            if (PropDecl) {
                FldName = PropDecl->Name->GetStr();
            }
            if (FldName == "") {
                FldTD = GetGlobalTypeDef(FldDecl->hDT);
                if (FldTD) {
                    FldName = Ansistrings::Format("(:%s)", FldTD->Name->GetStr());
                } else {
                    FldName = Ansistrings::Format("(@%d)", FldDecl->Ndx);
                }
            }
        }
        QS = Ansistrings::Format(".%s%s",FldName,QS);
    }*/

    while (DeclL) {
        Decl = DeclL;
        if (Decl->InheritsFrom(__classid(TCopyDecl))) Decl = static_cast<TCopyDecl *>(Decl)->Base;
        if (Decl->InheritsFrom(__classid(TLocalDecl)) && static_cast<TLocalDecl *>(Decl)->GetTag() == arFld) {
            int FldOfs = static_cast<TLocalDecl *>(Decl)->Ndx;
            if (FldOfs >= 0) {
                if (FldOfs <= Ofs) {
                    FldTD = GetGlobalTypeDef(static_cast<TLocalDecl *>(Decl)->hDT);
                    if (FldTD && Ofs < FldOfs + FldTD->Sz) {
                        FldName = Decl->Name->GetStr();
                        if (FldName == "") {
                            Decl = GetFldProperty(static_cast<TNameDecl *>(Decl), static_cast<TLocalDecl *>(Decl)->hDT);
                            if (Decl) FldName = Decl->Name->GetStr();
                            if (FldName == "") FldName = Sysutils::Format("(:%s)", ARRAYOFCONST((FldTD->Name->GetStr())));
                        }
                        return Sysutils::Format(".%s%s%s", ARRAYOFCONST((FldName, ShowOfsQualifier(((TLocalDecl *) Decl)->hDT, Ofs - FldOfs))));
                    }
                } else {
                    if (Sorted) break;
                }
            }
        }
        DeclL = DeclL->Next;
    }
    return "";
}

//------------------------------------------------------------------------------
// todo: review:
TLocalDecl *TRecBaseDef::GetFldByOfs(int Ofs, int QSz, int TotSize, bool Sorted) {
    if (Ofs >= TotSize) {
        return nullptr;
    }

    TLocalDecl *Result = nullptr;

    int dOfsBest  = System::MaxInt; // MaxInt
    int dRestBest = System::MaxInt; // MaxInt

    TDCURec  *DeclL = Fields;
    TDCURec  *Decl  = nullptr;
    TTypeDef *TD    = nullptr;

    int FldOfs = 0;
    int TSz    = 0;
    int dOfs   = 0;
    int dRest  = 0;

    while (DeclL) {
        Decl = DeclL;

        if (Decl->InheritsFrom(__classid(TCopyDecl))) {
            Decl = static_cast<TCopyDecl *>(Decl)->Base;
        }

        if (Decl && Decl->InheritsFrom(__classid(TLocalDecl))) {
            TLocalDecl *LD = static_cast<TLocalDecl *>(Decl);

            if ((LD->GetTag() == arFld) && ((LD->LocFlagsX & lfauxPropField) == 0)) {
                FldOfs = LD->Ndx;
                if (FldOfs >= 0) {
                    if (FldOfs <= Ofs) {
                        dOfs = Ofs - FldOfs;
                        if (dOfs <= dOfsBest) {
                            TD = GetGlobalTypeDef(LD->hDT);
                            if (TD) {
                                TSz = TD->Sz;
                            } else {
                                TSz = System::MaxInt - FldOfs;
                            }

                            if ((Ofs < FldOfs + TSz) && (Ofs + QSz <= FldOfs + TSz)) {
                                dRest = FldOfs + TSz - (Ofs + QSz);

                                if ((dOfs < dOfsBest) || (dRest < dRestBest)) {
                                    Result    = LD;
                                    dOfsBest  = dOfs;
                                    dRestBest = dRest;
                                    // break;
                                }
                            }
                        }
                    } else if (Sorted) {
                        break;
                    }
                }
            }
        }

        DeclL = DeclL->Next;
    }

    return Result;
}
//------------------------------------------------------------------------------
TMethodDecl *TRecBaseDef::GetMethodByVMTNDX(int VMTNDX, int VMTCnt) {
    if (VMTNDX >= VMTCnt) return nullptr;

    TDCURec *DeclL = Fields;
    while (DeclL) {
        TDCURec *Decl = DeclL;
        if (TCopyDecl *CD = dynamic_cast<TCopyDecl *>(Decl)) {
            CD = static_cast<TCopyDecl *>(CD->Base);
        }

        if (TMethodDecl *MD = dynamic_cast<TMethodDecl *>(Decl)) {
            // or lfOverride
            if ((MD->LocFlags & lfVirtual) != 0) {
                if (MD->hDT == VMTNDX) {
                    return MD;
                }
            }
        }

        DeclL = DeclL->Next;
    }
    return nullptr;
}
//------------------------------------------------------------------------------
TRecDef::TRecDef() : TRecBaseDef() {
    B2 = ReadByte();

    if (IsMSIL) {
        if (FVer >= verD2006 && FVer < verK1)
            Byte B1 = ReadByte();

        TNDX X = ReadUIndex();
        // !Temp Skip interface info - should make it stored in recs too
        ReadClassInterfaces(nullptr);
    } else if (FVer >= verD2005 && FVer < verK1) {
        if (FVer >= verD2006 && FVer < verK1) {
            Byte B1 = ReadByte();
            TNDX X0 = ReadByte();
        }
        TNDX X = ReadUIndex();
        if (FVer >= verD2009) {
            ReadUIndex();
            ReadUIndex();
            if (FVer >= verD2010)
                ReadUIndex();
        }
    }
    ReadFields(dlFields);
}
//------------------------------------------------------------------------------
void TRecDef::Visit(TDCURecVisitor *Visitor) { Visitor->visitRecDef(this); }
//------------------------------------------------------------------------------
int __fastcall TRecDef::ShowValue(Byte *DP, DWord DS, String &OutS) {
    return ShowFieldValues(DP, DS, OutS);
}
//------------------------------------------------------------------------------
// Check whether all the fields start where some previous field ends =>the record could be packed
bool __fastcall ChkIsPacked(TDCURec *L) {
    const int MaxFieldAlign = 8;

    int Ofs, Sz, SzRem;

    if (!L) return false; // no need for packing
    bool   PkRq = false;  // Without PkRq computing, almost any record will be packed
    TList *EndL = new TList;

    do {
        if (!L->InheritsFrom(__classid(TLocalDecl))) break;
        Ofs = static_cast<TLocalDecl *>(L)->Ndx;

        // The field starts at an unknown offset
        if (Ofs > 0 && EndL->IndexOf(reinterpret_cast<void *>(Ofs)) < 0) return false;
        Sz = GetTypeSize(static_cast<TLocalDecl *>(L)->hDT);

        // Unknown data type could be of any size => can't check packing
        if (Sz < 0) return false;

        SzRem = 0;
        if (Sz > 0) {
            SzRem = MaxFieldAlign;
            while (Sz % SzRem > 0) {
                SzRem /= 2;
            }
        }
        if (SzRem > 1 && Ofs % SzRem != 0) PkRq = true;
        Ofs += Sz;
        EndL->Add(reinterpret_cast<void *>(Ofs));
        L = L->Next;
    } while (L);
    delete EndL;
    return PkRq;
}
//------------------------------------------------------------------------------
// Find the smallest field offset, among the fields before the end of the previous field
int __fastcall GetCaseOfs(TDCURec *L) {
    int Ofs, Sz;

    int Result  = MAXINT;
    int PrevOfs = -1;

    while (L) {
        if (!L->InheritsFrom(__classid(TLocalDecl))) return Result;
        Ofs = static_cast<TLocalDecl *>(L)->Ndx;
        if (Ofs < PrevOfs) {
            if (Ofs < Result) Result = Ofs;
        }
        Sz = GetTypeSize(static_cast<TLocalDecl *>(L)->hDT);
        if (Sz < 0) Sz = 1; // For unknown data types I suppose that it should take some space
        PrevOfs = Ofs + Sz;
        L       = L->Next;
    }
    return Result;
}
//------------------------------------------------------------------------------
// Find 1st field >= OfsRq => 1st case field
TNameDecl **__fastcall GetNoCaseEP(TDCURec **L, int OfsRq) {
    while (*L && (*L)->InheritsFrom(__classid(TLocalDecl))) {
        if (static_cast<TLocalDecl *>(*L)->Ndx >= OfsRq) return reinterpret_cast<TNameDecl **>(L);
        L = &((*L)->Next);
    }
    return nullptr;
}
//------------------------------------------------------------------------------
// Requires: L-case field
// Find the next field with the same or higher (because of alignment) offset
// For example, in D7 this record has the following field offsets:
// TRec = record
//  case integer of
//  0: (A: integer@0);
//  1: (V: byte@0;
//    case integer of
//    0: (B: double@8);
//    1: (C: Byte@4))
// end ;
// function GetNextEP(var L: TDCURec; OfsRq: integer): PNameDecl;
TNameDecl **__fastcall GetNextEP(TDCURec *L, int OfsRq) {
    int Ofs;

    int Sz = GetTypeSize(static_cast<TLocalDecl *>(L)->hDT);
    if (Sz < 0) Sz = 1;

    int OfsMax = static_cast<TLocalDecl *>(L)->Ndx + Sz;

    TNameDecl **Result = reinterpret_cast<TNameDecl **>(&L->Next);
    while (*Result) {
        if (!(*Result)->InheritsFrom(__classid(TLocalDecl))) break;
        Ofs = static_cast<TLocalDecl *>(*Result)->Ndx;
        if (Ofs >= OfsRq && Ofs < OfsMax) return Result;
        Result = reinterpret_cast<TNameDecl **>(&((*Result)->Next));
    }
    return nullptr;
}
//------------------------------------------------------------------------------
// procedure ShowCase(Ofs0: Cardinal; Start: TDCURec{TNameDecl}; Sep: TDeclSepFlags; SK: TDeclSecKind);
void __fastcall ShowCase(TDCURec *Start, Byte SK) {
    TNameDecl *EP0 = nullptr;
    String     S;

    int CaseOfs = GetCaseOfs(Start);

    TNameDecl **EP = nullptr;

    if (CaseOfs < MAXINT) EP = GetNoCaseEP(&Start, CaseOfs);
    if (EP) {
        EP0 = *EP;
        *EP = nullptr;
    }

    ShowDeclList(dlFields, Start, S);

    if (EP) {
        int hCase;
        OutLog1("case Integer of\n");
        CaseN = hCase = 0;
        while (true) {
            *EP = EP0;
            OutLog2("%d:(", hCase); // The actual case labels and case data type are not stored in DCUs
            TNameDecl *CaseP = EP0;
            EP    = GetNextEP(CaseP, CaseOfs);
            if (EP) {
                EP0 = *EP;
                *EP = nullptr;
            }
            ShowCase(CaseP, SK);
            hCase++;
            CaseN = hCase;
            OutLog1(")\n");
            if (!EP) break;
        }
    }
}
//------------------------------------------------------------------------------
void __fastcall TRecDef::Show(String &OutS) {
    String S;
    if (ChkIsPacked(Fields)) OutLog1("packed ");
    OutLog1("record\n");
    TRecBaseDef::Show(S);
    ShowCase(Fields, skPublic);
    OutLog1("end");
    OutS = "";
}
//------------------------------------------------------------------------------
String __fastcall TRecDef::GetOfsQualifier(int Ofs) {
    return GetFldOfsQualifier(Ofs, Sz, false);
}
//------------------------------------------------------------------------------
TProcTypeDef::TProcTypeDef() : TRecBaseDef() {
    if (FVer > verD2)
        Ndx0 = ReadUIndex();

    hDTRes   = ReadUIndex();
    AddSz    = 0;
    AddStart = CurPos;
    Tag = ReadTag();

    // 99.99% that instead of WHILE it would be enough to use IF
    while (Tag != drEmbeddedProcStart) {
        if (Tag == drStop1) return;
        if (Byte CK = ReadCallKind(); CK == pcRegister) {
            if (FVer >= verD2009 && FVer < verK1) {
                Tag = ReadTag();

                switch (Tag) {
                    case drA5Info:
                        //Data.Bind.Components DXE3 Win64
                        break;
                    case drA7Info:
                        TTemplateParmsDeclModifier::Read(this);
                        break;
                    case drA8Info:
                        ReadUIndex();  //!!!M.b. some DCU record to be created
                        break;
                }
            }
        } else {
            CallKind = CK;
        }
        AddSz++;
    }
    ReadFields(dlArgsT);
}
//------------------------------------------------------------------------------
void TProcTypeDef::Visit(TDCURecVisitor *Visitor) { Visitor->visitProcTypeDef(this); }
//------------------------------------------------------------------------------
int __fastcall TProcTypeDef::ShowValue(Byte *DP, DWord DS, String &OutS) {
    OutS = "";
    if (Sz > DS) return -1;
    if (Sz == FPtrSize) {
        ShowPointer(DP, "Nil", OutS, nullptr);
        return Sz;
    }
    return TRecBaseDef::ShowValue(DP, Sz, OutS);
}
//------------------------------------------------------------------------------
bool __fastcall TProcTypeDef::IsProc() { return TypeIsVoid(hDTRes); }
//------------------------------------------------------------------------------
String __fastcall TProcTypeDef::ProcStr() {
    if (IsProc())
        return "procedure";
    else
        return "function";
}
//------------------------------------------------------------------------------
void __fastcall TProcTypeDef::ShowDecl(char *Braces, String &OutS) {
    String S;
    OutS = "";

    TRecBaseDef::Show(S);
    if (Fields) {
        OutS += Braces[0];
        OutLog2("%c", Braces[0]);
        ShowDeclList(dlArgsT, Fields, S);
        OutS += S;
        OutS += Braces[1];
        OutLog2("%c", Braces[1]);
    }
    if (!IsProc()) {
        OutS += ":";
        OutLog1(":");
        OutS += ShowTypeDef(hDTRes, NULL);
    }
    if ((Ndx0 & 0x10) != 0) {
        OutS += " of object";
        OutLog1(" of object");
    }
    if (CallKind != pcRegister) {
        S = CallKindName[CallKind];
        OutS += " " + S;
        OutLog2(" %s", S.c_str());
    }
}
//------------------------------------------------------------------------------
void __fastcall TProcTypeDef::Show(String &OutS) {
    // if (FVer>=verD13)and(FVer<verK1)and(NDX0 & ptfReference!=0) PutKWSp('reference to');
    String S;
    OutS = ProcStr();
    OutLog2("%s", OutS.c_str());
    ShowDecl("()", S);
    OutS += S;
}
//------------------------------------------------------------------------------
void __fastcall TProcTypeDef::EnumUsedTypes(TTypeUseAction Action, DWord *IP) {
    TRecBaseDef::EnumUsedTypes(Action, IP);
    if (!IsProc()) Action(this, hDTRes, IP);
}

//------------------------------------------------------------------------------
TTypeValKind TProcTypeDef::ValKind() {
    if ((Ndx0 & 0x10) != 0) {
        return TTypeValKind::vkMethod;
    }
    return TTypeValKind::vkPointer;
}
//------------------------------------------------------------------------------
void TOOTypeDef::Visit(TDCURecVisitor *Visitor) { Visitor->visitOOTypeDef(this); }
//------------------------------------------------------------------------------

/**
 * This is unused.
 *
 * @param Ofs
 * @return
 */
/*TMethodDecl * TOOTypeDef::GetMethodByVMTOfs(int Ofs) {
    if (!hasVMT())
        return nullptr;

    if ((Ofs & 0x3) != 0)
        return nullptr;

    Ofs /= 4;

    if (Ofs >= VMCnt)
        return nullptr;

    if (TMethodDecl *Result = GetMethodByVMTNDX(Ofs, VMCnt); Result || hParent == 0)
        return Result;

    TTypeDef *TD = GetGlobalTypeDef(hParent);
    TOOTypeDef *OOTD = dynamic_cast<TOOTypeDef *>(TD);

    if (OOTD == nullptr)
        return nullptr;

    return OOTD->GetMethodByVMTOfs(Ofs * 4);
}*/
//------------------------------------------------------------------------------
bool __fastcall TOOTypeDef::hasVMT() { return true; }
//------------------------------------------------------------------------------
TObjDef::TObjDef() : TOOTypeDef() {
    B03 = ReadByte();

    if (FVer >= verD2006 && FVer < verK1)
        TNDX BX = ReadUIndex();

    if (FVer >= verDXE1 && FVer < verK1)
        Byte BX1 = ReadByte();

    hParent = ReadUIndex();
    VMTOfs  = ReadUIndex();
    hVMT    = ReadIndex();
    VMCnt   = ReadIndex();

    if (FVer >= verDXE2 && FVer < verK1)
        TNDX BX2 = ReadUIndex();

    // BFE     = ReadByte();
    // Ndx1    = ReadIndex();
    // B00     = ReadByte();
    ReadFields(dlFields);
}
//------------------------------------------------------------------------------
void TObjDef::Visit(TDCURecVisitor *Visitor) { Visitor->visitObjDef(this); }
//------------------------------------------------------------------------------
int __fastcall TObjDef::ShowValue(Byte *DP, DWord DS, String &OutS) { return ShowFieldValues(DP, DS, OutS); }
//------------------------------------------------------------------------------
void __fastcall TObjDef::Show(String &OutS) {
    String S = "";
    OutLog1("object");
    TRecBaseDef::Show(S);
    if (hParent) OutLog2("(%s)", ShowTypeName(hParent).c_str());
    ShowDeclList(dlFields, Fields, S);
    OutLog1("end");
}
//------------------------------------------------------------------------------
void __fastcall TObjDef::EnumUsedTypes(TTypeUseAction Action, DWord *IP) {
    TRecBaseDef::EnumUsedTypes(Action, IP);
    if (hParent) Action(this, hParent, IP);
}
//------------------------------------------------------------------------------
TNDX __fastcall TObjDef::GetParentType() { return hParent; }
//------------------------------------------------------------------------------
String __fastcall TObjDef::GetOfsQualifier(int Ofs) {
    String Result = GetFldOfsQualifier(Ofs, Sz, true);
    if (Result != "") return Result;
    if (hParent) return ShowOfsQualifier(hParent, Ofs);
    return TRecBaseDef::GetOfsQualifier(Ofs);
}
//------------------------------------------------------------------------------
bool TObjDef::hasVMT() { return VMTOfs >= 0; }
//------------------------------------------------------------------------------
TClassDef::TClassDef() : TOOTypeDef() {
    TNDX Msk;

    if (FVer >= verD2006 && FVer < verK1)
        Byte BX = ReadByte(); // Some flags

    if (FVer >= verD2009 && FVer < verK1) {
        if (FVer >= verDXE2 && FVer < verK1)
            ReadByte();
        else
            ReadUIndex(); // It could be byte too, but it's to be checked // BX1

        ReadByte(); // BX2
    }

    hParent        = ReadUIndex();
    InstBaseRTTISz = ReadUIndex();
    InstBaseSz     = ReadIndex();
    InstBaseV      = ReadUIndex();
    VMCnt          = ReadUIndex();
    NdxFE          = ReadUIndex();
    PropCnt        = ReadUIndex();

    if (FVer >= verD8 && FVer < verK1) {
        Flags = ReadUIndex();
        Msk = 0x08;
    } else {
        Flags = ReadByte();
        Msk = 0x10;
    }

    if (FVer >= verD2010 && FVer < verK1)
        ReadUIndex(); // BX3

    if (FromPackage && ((Flags & Msk) > 0) && !IsMSIL) {
        ReadUIndex(); // Usually #1
        const int N = ReadUIndex(); // Usually #1
        for (int i = 1; i <= N; i++)
            ReadUIndex(); // Usually #1
    }

    if (FVer > verD2) {
        ReadBeforeIntf(); // For TMetaClassDef
        ICnt = ReadClassInterfaces(&ITbl);
    }

    ReadFields(dlClass);
    // MarkAuxFields();
}
//------------------------------------------------------------------------------
TClassDef::~TClassDef() {
    if (ITbl) delete[] ITbl;
}
//------------------------------------------------------------------------------
void TClassDef::Visit(TDCURecVisitor *Visitor) { Visitor->visitClassDef(this); }
//------------------------------------------------------------------------------
int __fastcall TClassDef::ShowValue(Byte *DP, DWord DS, String &OutS) {
    OutS = "";
    if (Sz > DS) return -1;
    if (Sz == FPtrSize) {
        ShowPointer(DP, "Nil", OutS, nullptr);
        return Sz;
    }
    return TRecBaseDef::ShowValue(DP, Sz, OutS);
}
//------------------------------------------------------------------------------
void __fastcall TClassDef::Show(String &OutS) {
    String S;
    OutS = "class";
    OutLog1("class");
    if (FVer >= verD8 && FVer < verK1) {
        if ((Flags & 0x04) != 0) {
            OutS += " abstract";
            OutLog1(" abstract");
        }
        if ((Flags & 0x40) != 0) {
            OutS += " sealed";
            OutLog1(" sealed");
        }
    }
    if (hParent || ICnt) {
        OutS += "(";
        OutLog1("(");
        int i = 0;
        if (hParent) {
            S = ShowTypeName(hParent);
            OutS += S;
            i++;
        }
        NDXHi = 0;
        for (int j = 0; j < ICnt; j++) {
            if (i > 0) {
                OutS += ",";
                OutLog1(",");
            }
            S = ShowTypeName(*ITbl[2 * j]);
            OutS += S;
        }
        OutS += ")";
        OutLog1(")");
    }
    OutLog2("VMCnt:%d\n", VMCnt);
    TRecBaseDef::Show(S);
    CaseN = -1;
    // dcu32: ShowDeclList(dlClass,Self{MainRec},Fields,Ofs0,2,[dsLast],ClassSecKinds[(CurUnit.Ver>=verD8)and(CurUnit.Ver<verK1)],skNone);
    ShowDeclList(dlClass, Fields, S);
    OutLog1("end");
}
//------------------------------------------------------------------------------
TNDX __fastcall TClassDef::GetParentType() { return hParent; }
//------------------------------------------------------------------------------
String __fastcall TClassDef::GetRefOfsQualifier(int Ofs) {
    String Result;

    Result = GetFldOfsQualifier(Ofs, InstBaseSz, true);
    if (Result != "") return Result;
    if (hParent)
        return ShowRefOfsQualifier(hParent, Ofs);
    else
        return TRecBaseDef::GetRefOfsQualifier(Ofs);
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void __fastcall TClassDef::ReadBeforeIntf() {}
//------------------------------------------------------------------------------
// todo: review
TLocalDecl *TClassDef::GetObjFldByOfs(int Ofs, int QSz, Byte *ObjUnit) {
    Byte *U = nullptr; // TUnit
    Byte *U0 = nullptr; // TUnit
    TTypeDef *TD = nullptr;

    TLocalDecl *Result = GetFldByOfs(Ofs, QSz, InstBaseSz, true);

    if ((Result != nullptr) || (hParent == 0)) {
        // ObjUnit = CurUnit; // CurUnit
        return Result;
    }

    ObjUnit = nullptr;
    TD = GetGlobalTypeDef(hParent);

    if ((!TD) || !(TD->InheritsFrom(__classid(TClassDef)))) {
        return nullptr;
    }

    // U0      = CurUnit;
    // CurUnit = U;

    try {
        Result = static_cast<TClassDef *>(TD)->GetObjFldByOfs(Ofs, QSz, ObjUnit);
    } __finally {
        // CurUnit = U0;
    }

    return Result;
}
//------------------------------------------------------------------------------
void TLocalDecl::ShowName(String &OutS) {
    // todo: review: necessary?
    /*auto ShowAuxFieldQualifier[]() {
        TLocalDecl *RefFld = reinterpret_cast<TLocalDecl *>(NdxB);

        if (RefFld) {
            RefFld->ShowName();
        }

        int TSz = GetTypeSize(hDT);
        if (TSz < 0) {
            TSz = 0;
        }

        int dOfs = NDX - (RefFld ? RefFld->Ndx : 0);

        TUnit *RefU = GetDCUOfMemory(RefFld != nullptr ? RefFld->Def : nullptr);
        if (!RefU) {
            if (dOfs > 0) {
                // PutSFmt(L"+%d", ARRAYOFCONST((dOfs)));
            }
        } else {
            AnsiString sQ;
            RefU->GetOfsQualifierEx(RefFld->hDT, dOfs, TSz, nullptr, &sQ);
            // PutS(sQ);
        }
    }*/

    // if (((LocFlagsX & lfauxPropField) != 0) && (NdxB != 0))
    //      ShowAuxFieldQualifier();
    // else
        TLocalDeclBase::ShowName(OutS);
}
//------------------------------------------------------------------------------

/**
 * Set the lfauxPropField flags
 */
void TClassDef::MarkAuxFields() {
    TDCURec   *DeclL    = Fields;
    TDCURec   *Decl     = nullptr;
    TPropDecl *PropDecl = nullptr;

    int TSz = 0;
    // TUnit     *FldUnit  = nullptr;

    while (DeclL) {
        Decl = DeclL;
        if (Decl->InheritsFrom(__classid(TLocalDecl))) {
            TLocalDecl *LD = static_cast<TLocalDecl *>(Decl);
            if ((LD->GetTag() == arFld) && (!LD->Name || LD->Name->IsEmpty())) {
                PropDecl = GetFldProperty(LD, LD->hDT);
                if (PropDecl) {
                    LD->LocFlagsX = LD->LocFlagsX | lfauxPropField;
                    TSz = GetTypeSize(LD->hDT);
                    if (TSz < 0) {
                        TSz = 0; // to fit anywhere
                    }
                    TLocalDecl *temp = reinterpret_cast<TLocalDecl*>(LD->NdxB);
                    temp = GetObjFldByOfs(LD->Ndx, TSz, nullptr);
                }
            }
        }

        DeclL = DeclL->Next;
    }
}
//------------------------------------------------------------------------------
TMetaClassDef::TMetaClassDef() : TClassDef() {}
//------------------------------------------------------------------------------
void TMetaClassDef::Visit(TDCURecVisitor *Visitor) { Visitor->visitMetaClassDef(this); }
//------------------------------------------------------------------------------
void __fastcall TMetaClassDef::ReadBeforeIntf() {
    hCl = ReadUIndex();
    ReadUIndex(); // Ignore - was always 0
}
//------------------------------------------------------------------------------
TInterfaceDef::TInterfaceDef() : TOOTypeDef() {
    Byte LK;

    if (FVer >= verD2009 && FVer < verK1) {
        ReadByte(); // ReadUIndex();
    }

    hParent = ReadUIndex();
    VMCnt   = ReadIndex();
    GUID    = reinterpret_cast<PGUID>(ReadMem(16)); // sizeof(TGUID)
    B       = ReadByte();

    if ((B & 4) == 0)
        LK = dlInterface;
    else
        LK = dlDispInterface;

    if (FVer >= verD8 && FVer < verK1) {
        if (FVer >= verD2010 && FVer < verK1)
            ReadUIndex();

        int Cnt = ReadUIndex();
        for (int i = 1; i <= Cnt; i++) {
            ReadUIndex();
            ReadUIndex();
            if (IsMSIL && FVer >= verD2006 && FVer < verK1) {
                ReadUIndex();
                ReadUIndex();
            }
        }

        // ReadClassInterfaces(NULL);
    }
    ReadFields(LK);
}
//------------------------------------------------------------------------------
void TInterfaceDef::Visit(TDCURecVisitor *Visitor) { Visitor->visitInterfaceDef(this); }
//------------------------------------------------------------------------------
void __fastcall TInterfaceDef::Show(String &OutS) {
    char   guid[1024];
    String S, S1;
    OutS = "interface ";
    OutLog1("interface ");
    if (hParent) {
        S = ShowTypeName(hParent);
        OutS += "(" + S + ") ";
        OutLog2("(%s)", S.c_str());
    }
    OutLog2("VMCnt:%d\n", VMCnt);
    TRecBaseDef::Show(S);
    sprintf(guid, "['{%8.8lX-%4.4X-%4.4X-%2.2X%2.2X-%2.2X%2.2X%2.2X%2.2X%2.2X%2.2X}']", GUID->Data1, GUID->Data2,
            GUID->Data3, GUID->Data4[0], GUID->Data4[1], GUID->Data4[2], GUID->Data4[3], GUID->Data4[4], GUID->Data4[5],
            GUID->Data4[6], GUID->Data4[7]);
    OutS += String(guid);
    OutLog2("%s\n", guid);
    ShowDeclList(dlInterface, Fields, S);
    OutLog1("end");
}
//------------------------------------------------------------------------------
TVoidDef::TVoidDef() : TTypeDef() {
    if (FVer >= verDXE1 && FVer < verK1) int X = ReadUIndex();
}
//------------------------------------------------------------------------------
void TVoidDef::Visit(TDCURecVisitor *Visitor) { Visitor->visitVoidDef(this); }
//------------------------------------------------------------------------------
void __fastcall TVoidDef::Show(String &OutS) {
    String SType;
    OutS = "void";
    OutLog1("void");
    TTypeDef::Show(SType);
    OutS += SType;
}
//------------------------------------------------------------------------------
TA6Def::TA6Def() : TDCURec() {
    Tag = ReadTag();
    ReadDeclList(dlA6, nullptr, &Args);
    if (Tag != drStop1) printf("Stop Tag\n");
}
//------------------------------------------------------------------------------
TA6Def::~TA6Def() { FreeDCURecList(Args); }
//------------------------------------------------------------------------------
void TA6Def::Visit(TDCURecVisitor *Visitor) { Visitor->visitA6Def(this); }
//------------------------------------------------------------------------------
void __fastcall TA6Def::Show(String &OutS) { ShowDeclList(dlA6, Args, OutS); }
//------------------------------------------------------------------------------
/*TA7Def::TA7Def() : TDCURec() {
    hClass = ReadUIndex();
    Cnt    = ReadUIndex();
    Tbl    = new int[Cnt];
    for (int i = 0; i < Cnt; i++) {
        Tbl[i] = ReadUIndex();
    }
}
//------------------------------------------------------------------------------
TA7Def::~TA7Def() {
    if (Tbl) delete[] Tbl;
}
//------------------------------------------------------------------------------
void TA7Def::Visit(TDCURecVisitor *Visitor) { Visitor->visit }
//------------------------------------------------------------------------------
void __fastcall TA7Def::Show(String &OutS) {
    OutS = "";
    OutLog1("A7");
    char Sep = '[';
    for (int i = 0; i < Cnt; i++) {
        OutLog3("%s#%x", Sep, Tbl[i]);
        Sep = ',';
    }
    OutLog1("]");
}*/
//------------------------------------------------------------------------------
TDelayedImpRec::TDelayedImpRec() : TNameDecl(true) {
    Inf = ReadULong();
    F   = ReadUIndex();
    RefAddrDef(F);
}
//------------------------------------------------------------------------------
void TDelayedImpRec::Visit(TDCURecVisitor *Visitor) { Visitor->visitDelayedImpRec(this); }
//------------------------------------------------------------------------------
void __fastcall TDelayedImpRec::Show(String &OutS) {
    TNameDecl::Show(OutS);
    OutLog1("B0");
    OutLog3("{%x,#%x}", Inf, F);
    OutS = "";
}
//------------------------------------------------------------------------------
TORecDecl::TORecDecl() : TNameDecl(true) {
    DW  = ReadULong();
    B0  = ReadByte();
    B1  = ReadByte();
    Tag = ReadTag();
    ReadDeclList(dlA6, nullptr, &Args);
    if (Tag != drStop1) printf("Stop Tag\n");
}
//------------------------------------------------------------------------------
TORecDecl::~TORecDecl() { FreeDCURecList(Args); }
//------------------------------------------------------------------------------
void TORecDecl::Visit(TDCURecVisitor *Visitor) { Visitor->visitORecDecl(this); }
//------------------------------------------------------------------------------
void __fastcall TORecDecl::Show(String &OutS) {
    TNameDecl::Show(OutS);
    OutLog1("ORec");
    ShowDeclList(dlA6, Args, OutS);
    OutS = "";
}
//------------------------------------------------------------------------------
TDynArrayDef::TDynArrayDef() : TPtrDef() {}
//------------------------------------------------------------------------------
void TDynArrayDef::Visit(TDCURecVisitor *Visitor) { Visitor->visitDynArrayDef(this); }
//------------------------------------------------------------------------------
void __fastcall TDynArrayDef::Show(String &OutS) {
    ShowBase();
    TTypeDef *TD = GetGlobalTypeDef(hRefDT);
    if (TD && TD->InheritsFrom(__classid(TArrayDef0))) {
        OutS = "array of";
        OutLog1("array of");
        OutS += ShowTypeDef(((TArrayDef0 *) TD)->hDTEl, NULL);
    } else
        TPtrDef::Show(OutS);
}
//------------------------------------------------------------------------------
String __fastcall TDynArrayDef::GetRefOfsQualifier(int Ofs) {
    if (Ofs == -4) return ".Length";
    if (Ofs == -8) return ".RefCnt";
    return TPtrDef::GetRefOfsQualifier(Ofs);
}
//------------------------------------------------------------------------------
TTemplateArgDef::TTemplateArgDef() : TTypeDef() {
    Cnt = ReadUIndex();
    // Tbl = new int[Cnt];
    // Tbl = AllocMem(Cnt * sizeof(TNDX))
    Tbl = new TNDXTbl[Cnt];
    for (int i = 0; i < Cnt; i++) {
        *Tbl[i] = ReadUIndex();
    }
    V5 = ReadUIndex();
}
//------------------------------------------------------------------------------
TTemplateArgDef::~TTemplateArgDef() {
    if (Tbl) delete[] Tbl;
}
//------------------------------------------------------------------------------
void TTemplateArgDef::Visit(TDCURecVisitor *Visitor) { Visitor->visitTemplateArgDef(this); }
//------------------------------------------------------------------------------
void __fastcall TTemplateArgDef::Show(String &OutS) {
    TTypeDef::Show(OutS);
    OutLog1("template arg");
    if (V5 > 0) OutLog2(" #%x", V5);
    if (Cnt > 0) {
        char Sep = '<';
        for (int i = 0; i < Cnt; i++) {
            OutLog3("%s#%x", Sep, Tbl[i]);
            Sep = ',';
        }
        OutLog1(">");
    }
}
//------------------------------------------------------------------------------
TTemplateCall::TTemplateCall() : TTypeDef() {
    if (FVer >= verDXE1 && FVer < verK1)
        int X = ReadByte(); // ReadUIndex() - it was detected in verD_XE2 and Ok for verD_XE

    hDT  = ReadUIndex();
    Cnt  = ReadUIndex();
    // Args = new int[Cnt]; //  AllocMem(Cnt*SizeOf(TNDX));
    Args = new TNDXTbl[Cnt];
    for (int i = 0; i < Cnt; i++) {
        *Args[i] = ReadUIndex();
        if (FVer >= verD12 && FVer < verK1) {
            ReadSomeNameInfo28();
        }
    }
    hDTFull = ReadUIndex();
    //!!! FixDTName();
}
//------------------------------------------------------------------------------
TTemplateCall::~TTemplateCall() {
    if (Args) delete[] Args;
    FreeName(FixedName);
}
//------------------------------------------------------------------------------
void TTemplateCall::Visit(TDCURecVisitor *Visitor) { Visitor->visitTemplateCall(this); }
//------------------------------------------------------------------------------
void __fastcall TTemplateCall::Show(String &OutS) {
    TTypeDef::Show(OutS);
    if (hDTFull) OutS += ShowTypeName(hDTFull);
    OutS += ShowTypeName(hDT);

    // needed?
    /*if Writer.AuxLevel<=0 then begin
      RemOpen;
      //CurUnit.ShowTypeName(hDT);
      if (FixedName<>Nil)and((OldName=Nil)or(not FixedName^.Eq(OldName){FixedName^<>OldName^})) then begin
        PutCh('|');
        PutS(FixedName^.GetStr);
      end ;
      RemClose;
      SoftNL;
    end ;*/

    char Sep = '<';
    for (int i = 0; i < Cnt; i++) {
        OutS += String(Sep);
        OutLog2("%c", Sep);
        Sep = ',';
        OutS += ShowTypeDef(*Args[i], nullptr);
    }
    OutLog1(">");
    OutS += ">";

    // if (hDTFull<>0)
    // AuxRemClose();
}
//------------------------------------------------------------------------------
/**
 *
 * @param DP
 * @param DS
 * @param OutS
 * @return Size used
 */
int __fastcall TTemplateCall::ShowValue(Byte *DP, DWord DS, String &OutS) {
    int Result = ShowGlobalTypeValue(hDT, DP, DS, false, -1, false, OutS);

    if (Result > 0) {
        Result = TTypeDef::ShowValue(DP, DS, OutS);
    }

    return Result;
;}
//------------------------------------------------------------------------------
void __fastcall TTemplateCall::EnumUsedTypes(TTypeUseAction Action, DWord *IP) { Action(this, hDT, IP); }
//------------------------------------------------------------------------------
TTypeValKind TTemplateCall::ValKind() { return GetGlobalTypeValKind(hDT); }
//------------------------------------------------------------------------------
void TTemplateCall::FixDTName() {
    TTypeDef *TD = GetLocalTypeDef(hDT);
    if (!TD) return;
    if (!OldName) return;
    AnsiString S = OldName->GetStr();
    char* EP = Ansistrings::StrScan(S.c_str(), '`');
    if (!EP) return;
    S.SetLength(EP - S);
    char Sep = '<';
    for (int i=0; i<Cnt; i++) {
        PName NP = GetTypeName(*Args[i]);
        if (!NP) return;
        S += Sep + NP->GetStr();
        Sep = ',';
    }
    S += '>';
    FixedName = AllocName(S);
    TD->FName = FixedName;
}
//------------------------------------------------------------------------------
// todo: review
TAssemblyData::TAssemblyData() {
    HdrSz = ReadUIndex();
    TIncPtr HdrStart = CurPos;
    F = ReadULong();
    SzPublicKey = ReadULong();
    PublicKey = ReadMem(SzPublicKey);
    SzPublicKeyToken = ReadULong();
    PublicKeyToken = ReadMem(SzPublicKeyToken);
    Y = ReadULong();

    DWord Sz = ReadULong(); // ulong

    AssemblyName = reinterpret_cast<char *>(ReadMem(Sz));
    // AssemblyName = ReadMem((Sz+3)and not $3); // align on 4b boundary
    SomeData = ReadMem(0x18);
    Sz       = CurPos - HdrStart;

    if ((Sz > HdrSz) || (Sz + 8 < HdrSz)) {
        printf("[Error] Unexpected AssemblyData header size $%x\n", HdrSz); // DCUErrorFmt
    }

    SkipBlock(HdrSz-Sz);
    Descr = ReadShortName();
    Cnt1 = ReadUIndex();
    Tbl1 = reinterpret_cast<PulongTbl>(ReadMem(Cnt1 * sizeof(DWord))); // ulong
    Tbl2 = static_cast<PulongTbl>(AllocMem(Cnt1 * sizeof(DWord))); // ulong

     for (int i = 0; i < Cnt1; i++) {
         *Tbl2[i] = ReadUIndex();
     }

    Cnt2 = ReadUIndex();
    Tbl3 = reinterpret_cast<PulongTbl>(ReadMem(Cnt2 * sizeof(DWord))); // ulong
    Tbl4 = reinterpret_cast<PulongTbl>(ReadMem(Cnt2 * sizeof(DWord))); // ulong
    Tbl5 = reinterpret_cast<PulongTbl>(ReadMem(Cnt2 * sizeof(DWord))); // ulong
    Cnt3 = ReadUIndex();
    Tbl6 = reinterpret_cast<PulongTbl>(ReadMem(Cnt3 * sizeof(DWord))); // ulong
}
//------------------------------------------------------------------------------
TAssemblyData::~TAssemblyData() {
    if (Tbl2) {
        delete[] Tbl2;
    }
}
//------------------------------------------------------------------------------
void TAssemblyData::Visit(TDCURecVisitor *Visitor) { Visitor->visitAssemblyData(this); }
//------------------------------------------------------------------------------
void __fastcall TAssemblyData::Show(String &OutS) {
    // todo
    // PutKWSp('AssemblyData');
    // PutSFmt('(#%d,%x',[HdrSz,F]);
    // ShiftNLOfs(2);
    // try
    //     NL;
    // PutS('PublicKey:');
    // PutS(DumpStr(PublicKey^,SzPublicKey));
    // NL;
    // PutS('AssemblyName: ');
    // PutS(AssemblyName);
    // NL;
    // PutS('SomeData:');
    // PutS(DumpStr(SomeData^,$18));
    // NL;
    // PutS('Descr: ');
    // PutS(Descr^);
    // NL;
    // PutS('Tbl1: ');
    // ShowUlongTbl(Tbl1,Cnt1);
    // NL;
    // PutS('Tbl2: ');
    // ShowLongTbl(Tbl2,Cnt1);
    // NL;
    // PutS('Tbl3: ');
    // ShowUlongTbl(Tbl3,Cnt2);
    // NL;
    // PutS('Tbl4: ');
    // ShowUlongTbl(Tbl4,Cnt2);
    // NL;
    // PutS('Tbl5: ');
    // ShowUlongTbl(Tbl5,Cnt2);
    // NL;
    // PutS('Tbl6: ');
    // ShowUlongTbl(Tbl6,Cnt3);
    // NL;
    // finally
    //     ShiftNLOfs(-2);
    // end ;
    // PutCh(')');
}
//------------------------------------------------------------------------------
bool __fastcall TAssemblyData::IsVisible(Byte LK) {
    // TDeclListKind::Main
    return LK != dlMain; // Show in implementation or other places
}

/*procedure ShowUlongTblEx(Tbl: PulongTbl; Cnt: Integer; const ValFmt: AnsiString);
var
  i: Integer;
begin
  PutCh('(');
  ShiftNLOfs(2);
  try
    for i:=0 to Cnt-1 do begin
      if i>0 then
        PutS(','+cSoftNL);
      PutSFmt('%d:',[i]);
      PutSFmt(ValFmt,[Tbl^[i]]);
    end ;
  finally
    ShiftNLOfs(-2);
  end ;
  PutCh(')');
end;

procedure ShowUlongTbl(Tbl: PulongTbl; Cnt: Integer);
begin
  ShowUlongTblEx(Tbl,Cnt,'$%x');
end;

procedure ShowLongTbl(Tbl: PulongTbl; Cnt: Integer);
begin
  ShowUlongTblEx(Tbl,Cnt,'#%d');
end;*/

//------------------------------------------------------------------------------
void __fastcall TDCURecVisitor::doVisit(TDCURec *DCURec) {
    bool SaveVisited = FVisited;
    SaveVisited = FVisited;
    FVisited = true;
    DCURec->Visit(this);
    if (FVisited) afterVisit(DCURec);
    FVisited = SaveVisited;
}
//------------------------------------------------------------------------------
void __fastcall TDCURecVisitor::afterVisit(TDCURec *DCURec) {}

void __fastcall TDCURecVisitor::visitDCURec(TDCURec *DCURec) {}

void __fastcall TDCURecVisitor::visitBaseDef(TBaseDef *BaseDef) { visitDCURec(BaseDef); }

void __fastcall TDCURecVisitor::visitImpDef(TImpDef *ImpDef) { visitBaseDef(ImpDef); }

void __fastcall TDCURecVisitor::visitUnitImpDef(TUnitImpDef *UnitImpDef) { visitImpDef(UnitImpDef); }

void __fastcall TDCURecVisitor::visitDLLImpRec(TDLLImpRec *DLLImpRec) { visitBaseDef(DLLImpRec); }

void __fastcall TDCURecVisitor::visitImpTypeDefRec(TImpTypeDefRec *ImpTypeDefRec) { visitImpDef(ImpTypeDefRec); }

void __fastcall TDCURecVisitor::visitNameDecl(TNameDecl *NameDecl) { visitDCURec(NameDecl); }

void __fastcall TDCURecVisitor::visitNameFDecl(TNameFDecl *NameFDecl) { visitNameDecl(NameFDecl); }

void __fastcall TDCURecVisitor::visitTypeDecl(TTypeDecl *TypeDecl) { visitNameFDecl(TypeDecl); }

void __fastcall TDCURecVisitor::visitVarDecl(TVarDecl *VarDecl) { visitNameFDecl(VarDecl); }

void __fastcall TDCURecVisitor::visitVarVDecl(TVarVDecl *VarVDecl) { visitVarDecl(VarVDecl); }

void __fastcall TDCURecVisitor::visitVarCDecl(TVarCDecl *VarCDecl) { visitVarDecl(VarCDecl); }

void __fastcall TDCURecVisitor::visitAbsVarDecl(TAbsVarDecl *AbsVarDecl) { visitVarDecl(AbsVarDecl); }

void __fastcall TDCURecVisitor::visitTypePDecl(TTypePDecl *TypePDecl) { visitVarCDecl(TypePDecl); }

void __fastcall TDCURecVisitor::visitThreadVarDecl(TThreadVarDecl *ThreadVarDecl) { visitVarDecl(ThreadVarDecl); }

void __fastcall TDCURecVisitor::visitMemBlockRef(TMemBlockRef *MemBlockRef) { visitNameFDecl(MemBlockRef); }

void __fastcall TDCURecVisitor::visitStrConstDecl(TStrConstDecl *StrConstDecl) { visitMemBlockRef(StrConstDecl); }

void __fastcall TDCURecVisitor::visitLabelDecl(TLabelDecl *LabelDecl) { visitNameDecl(LabelDecl); }

void __fastcall TDCURecVisitor::visitExportDecl(TExportDecl *ExportDecl) { visitNameDecl(ExportDecl); }

void __fastcall TDCURecVisitor::visitLocalDecl(TLocalDecl *LocalDecl) { visitNameDecl(LocalDecl); }

void __fastcall TDCURecVisitor::visitMethodDecl(TMethodDecl *MethodDecl) { visitLocalDecl(MethodDecl); }

void __fastcall TDCURecVisitor::visitClassVarDecl(TClassVarDecl *ClassVarDecl) { visitLocalDecl(ClassVarDecl); }

void __fastcall TDCURecVisitor::visitPropDecl(TPropDecl *PropDecl) { visitNameDecl(PropDecl); }

void __fastcall TDCURecVisitor::visitDispPropDecl(TDispPropDecl *DispPropDecl) { visitLocalDecl(DispPropDecl); }

void __fastcall TDCURecVisitor::visitConstDeclBase(TConstDeclBase *ConstDeclBase) { visitNameFDecl(ConstDeclBase); }

void __fastcall TDCURecVisitor::visitConstDecl(TConstDecl *ConstDecl) { visitConstDeclBase(ConstDecl); }

void __fastcall TDCURecVisitor::visitResStrDef(TResStrDef *ResStrDef) { visitVarCDecl(ResStrDef); }

void __fastcall TDCURecVisitor::visitSetDeftInfo(TSetDeftInfo *SetDeftInfo) { visitDCURec(SetDeftInfo); }

void __fastcall TDCURecVisitor::visitCopyDecl(TCopyDecl *CopyDecl) { visitNameDecl(CopyDecl); }

void __fastcall TDCURecVisitor::visitProcDecl(TProcDecl *ProcDecl) { visitMemBlockRef(ProcDecl); }

void __fastcall TDCURecVisitor::visitSysProcDecl(TSysProcDecl *SysProcDecl) { visitNameDecl(SysProcDecl); }

void __fastcall TDCURecVisitor::visitSysProc8Decl(TSysProc8Decl *SysProc8Decl) { visitProcDecl(SysProc8Decl); }

void __fastcall TDCURecVisitor::visitUnitAddInfo(TUnitAddInfo *UnitAddInfo) { visitNameFDecl(UnitAddInfo); }

void __fastcall TDCURecVisitor::visitSpecVar(TSpecVar *SpecVar) { visitVarDecl(SpecVar); }

void __fastcall TDCURecVisitor::visitTypeDef(TTypeDef *TypeDef) { visitBaseDef(TypeDef); }

void __fastcall TDCURecVisitor::visitRangeBaseDef(TRangeBaseDef *RangeBaseDef) { visitTypeDef(RangeBaseDef); }

void __fastcall TDCURecVisitor::visitRangeDef(TRangeDef *RangeDef) { visitRangeBaseDef(RangeDef); }

void __fastcall TDCURecVisitor::visitEnumDef(TEnumDef *EnumDef) { visitRangeBaseDef(EnumDef); }

void __fastcall TDCURecVisitor::visitFloatDef(TFloatDef *FloatDef) { visitTypeDef(FloatDef); }

void __fastcall TDCURecVisitor::visitPtrDef(TPtrDef *PtrDef) { visitTypeDef(PtrDef); }

void __fastcall TDCURecVisitor::visitTextDef(TTextDef *TextDef) { visitTypeDef(TextDef); }

void __fastcall TDCURecVisitor::visitFileDef(TFileDef *FileDef) { visitTypeDef(FileDef); }

void __fastcall TDCURecVisitor::visitSetDef(TSetDef *SetDef) { visitTypeDef(SetDef); }

void __fastcall TDCURecVisitor::visitArrayDef0(TArrayDef0 *ArrayDef0) { visitTypeDef(ArrayDef0); }

void __fastcall TDCURecVisitor::visitArrayDef(TArrayDef *ArrayDef) { visitArrayDef0(ArrayDef); }

void __fastcall TDCURecVisitor::visitShortStrDef(TShortStrDef *ShortStrDef) { visitArrayDef(ShortStrDef); }

void __fastcall TDCURecVisitor::visitStringDef(TStringDef *StringDef) { visitArrayDef0(StringDef); }

void __fastcall TDCURecVisitor::visitVariantDef(TVariantDef *VariantDef) { visitTypeDef(VariantDef); }

void __fastcall TDCURecVisitor::visitObjVMTDef(TObjVMTDef *ObjVMTDef) { visitTypeDef(ObjVMTDef); }

void __fastcall TDCURecVisitor::visitRecBaseDef(TRecBaseDef *RecBaseDef) { visitTypeDef(RecBaseDef); }

void __fastcall TDCURecVisitor::visitRecDef(TRecDef *RecDef) { visitRecBaseDef(RecDef); }

void __fastcall TDCURecVisitor::visitProcTypeDef(TProcTypeDef *ProcTypeDef) { visitRecBaseDef(ProcTypeDef); }

void __fastcall TDCURecVisitor::visitOOTypeDef(TOOTypeDef *OOTypeDef) { visitRecBaseDef(OOTypeDef); }

void __fastcall TDCURecVisitor::visitObjDef(TObjDef *ObjDef) { visitOOTypeDef(ObjDef); }

void __fastcall TDCURecVisitor::visitClassDef(TClassDef *ClassDef) { visitOOTypeDef(ClassDef); }

void __fastcall TDCURecVisitor::visitMetaClassDef(TMetaClassDef *MetaClassDef) { visitClassDef(MetaClassDef); }

void __fastcall TDCURecVisitor::visitInterfaceDef(TInterfaceDef *InterfaceDef) { visitOOTypeDef(InterfaceDef); }

void __fastcall TDCURecVisitor::visitVoidDef(TVoidDef *VoidDef) { visitTypeDef(VoidDef); }

void __fastcall TDCURecVisitor::visitA6Def(TA6Def *A6Def) { visitDCURec(A6Def); }

void __fastcall TDCURecVisitor::visitDelayedImpRec(TDelayedImpRec *DelayedImpRec) { visitNameDecl(DelayedImpRec); }

void __fastcall TDCURecVisitor::visitORecDecl(TORecDecl *ORecDecl) { visitNameDecl(ORecDecl); }

void __fastcall TDCURecVisitor::visitDynArrayDef(TDynArrayDef *DynArrayDef) { visitPtrDef(DynArrayDef); }

void __fastcall TDCURecVisitor::visitTemplateArgDef(TTemplateArgDef *TemplateArgDef) { visitTypeDef(TemplateArgDef); }

void __fastcall TDCURecVisitor::visitTemplateCall(TTemplateCall *TemplateCall) { visitTypeDef(TemplateCall); }

void __fastcall TDCURecVisitor::visitAssemblyData(TAssemblyData *AssemblyData) { visitDCURec(AssemblyData); }

//------------------------------------------------------------------------------

bool TNameRec::IsEmpty() {
    return !this || D.bLen == 0 || this == &NoName;
}

void __fastcall TNameRec::GetStrInfo(TAnsiStrRec& SR) {
    if (!this) {
        SR.CP = nullptr;
        SR.Len = 0;
        return;
    }

    DWord L = D.bLen;

    if (L == 0xFF && FVer >= verDXE2 && FVer < verK1) {
        printf("Debug: TNameRec::GetStrInfo: over 255\n");
        SR.CP = reinterpret_cast<PAnsiChar>(&D.lS);
        SR.Len = D.dwLen;
    } else {
        // ShortString is 1-indexed in Pascal, so S[1] maps to S.c_str()[0] or &S[1] in C++Builder
        SR.CP = &D.S[1];
        SR.Len = L;
    }
}

AnsiChar __fastcall TNameRec::Get1stChar() {
    TAnsiStrRec SR{};
    GetStrInfo(SR);
    if (static_cast<int>(SR.Len) <= 0) {
        return '\0';
    }
    return SR.CP[0];
}

AnsiString __fastcall TNameRec::GetStr() {
    TAnsiStrRec SR{};
    GetStrInfo(SR);

    // SetString(Result, SR.CP, SR.Len)
    if (SR.CP && SR.Len > 0) {
        return AnsiString(SR.CP, SR.Len);
    }
    return "";
}

AnsiString __fastcall TNameRec::GetRightStr(std::int32_t dl) {
    if (!this) {
        return "";
    }

    TAnsiStrRec SR{};
    GetStrInfo(SR);

    int L = static_cast<int>(SR.Len) - dl;
    if (L <= 0) {
        return "";
    }

    // SetString(Result,SR.CP+dl,L);
    return AnsiString(SR.CP + dl, L);
}

bool __fastcall TNameRec::Eq(PName N) {
    if (!this || !N) {
        return N == this;
    }

    std::int32_t L = D.bLen;
    if (L != N->D.bLen) {
        return false;
    }

    PAnsiChar CP = nullptr;
    PAnsiChar CP1 = nullptr;

    if (L == 0xFF && FVer >= verDXE2 && FVer < verK1) {
        L = D.dwLen;
        if (L != N->D.dwLen) {
            return false;
        }
        CP = reinterpret_cast<PAnsiChar>(D.lS);
        CP1 = reinterpret_cast<PAnsiChar>(N->D.lS);
    } else {
        CP = reinterpret_cast<PAnsiChar>(&D.S[1]);
        CP1 = reinterpret_cast<PAnsiChar>(&N->D.S[1]);
    }

    return CompareMem(CP, CP1, L);
}

bool __fastcall TNameRec::EqS(const ShortString& S) {
    return Eq(reinterpret_cast<PName>(const_cast<ShortString*>(&S)));
}

bool __fastcall TNameRec::HasChar(AnsiChar ch) {
    TAnsiStrRec SR{};
    GetStrInfo(SR);

    for (std::uint32_t i = 0; i < SR.Len; ++i) {
        if (SR.CP[i] == ch) {
            return true;
        }
    }
    return false;
}

/**
 * The name is aux and shouldn't be shown if not requested
 * @return boolean
 */
bool __fastcall TNameRec::IsAuxName() {
    AnsiChar ch = Get1stChar();
    if (ch == '.') {
        return true;
    }

    if (FVer >= verD2009 && FVer < verK1) {
        if (ch == ':') {
            return true;
        }
        if (HasChar('`')) {
            return true;
        }
    }

    return false;
}
// int
static LongInt __fastcall GetStrRecHash(const TAnsiStrRec& SR) {
    LongInt Result = 0;
    PAnsiChar CP = SR.CP;

    for (Cardinal i = 0; i < SR.Len; ++i) {
        Result = Result * 17 + static_cast<Byte>(*CP);
        ++CP;
    }

    return Result;
}

// int
LongInt __fastcall TNameRec::GetHash() {
    TAnsiStrRec SR{};
    GetStrInfo(SR);
    return GetStrRecHash(SR);
}

// int
LongInt __fastcall TNameRec::GetRightHash(Integer Ofs) {
    TAnsiStrRec SR{};
    GetStrInfo(SR);

    if (Ofs < 0) {
        Ofs = 0;
    }

    if (static_cast<Cardinal>(Ofs) >= SR.Len) {
        SR.CP = nullptr;
        SR.Len = 0;
    } else {
        SR.CP += Ofs;
        SR.Len -= Ofs;
    }

    return GetStrRecHash(SR);
}
