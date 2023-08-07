unit KB;

interface

uses
  classes, SysUtils, DasmDefs, DCU_In, DCU_Out, FixUp, DCURecs, DCU32, DCP;

type
  MODULE_INFO = record
    ID:Integer;
    ModuleID:WORD;
    Offset:Cardinal;
    Size:Cardinal;
    Name:string;//Unit Name
    Filename:string;//Unit Filename
    UsesList:TStringList;//List of Uses
  end;
  PMODULE_INFO = ^MODULE_INFO;

  PROPERTY_INFO = record
    Scope:TDeclSecKind;
    Index:Integer;
    FDispId:Integer;
    Name:String;
    TypeDef:String;
    ReadName:String;
    WriteName:String;
    StoredName:String;
  end;
  PPROPERTY_INFO = ^PROPERTY_INFO;

  METHODDECL_INFO = record
    Scope:TDeclSecKind;
    MethodKind:Char;     //'M'-method,'P'-procedure,'F'-function,'C'-constructor,'D'-destructor
    Prototype:String;
  end;
  PMETHODDECL_INFO = ^METHODDECL_INFO;

  LOCALDECL_INFO = record
    Scope:TDeclSecKind;
    Tag:BYTE;
    LocFlags:Integer;
    Ndx:Integer;
    NdxB:Integer;
    FCase:Integer;           //for case
    Name:String;
    TypeDef:String;
    AbsName:String;
  end;
  PLOCALDECL_INFO = ^LOCALDECL_INFO;

  //Verify
  CONST_INFO = record
    ID:Integer;
    Offset:Cardinal;
    Size:Cardinal;
    Skip:Boolean;
    ModuleID:WORD;
    Name:String;
    FType:Char;       //look above
    TypeDef:String;
    Value:String;
    RTTISz:Cardinal;  //Size of RTTI data
    RTTIOfs:Cardinal; //Offset of RTTI data
    Fixups:TList;     //If VMT
  end;
  PCONST_INFO = ^CONST_INFO;

  TYPE_INFO = record
    ID:Integer;
    Offset:Cardinal;
    Size:Cardinal;
    ModuleID:WORD;
    Name:string;
    Kind:BYTE;
    VMCnt:WORD;       //Number of class VM
    RTTISz:Cardinal;  //Size of RTTI data
    RTTIOfs:Cardinal; //Offset of RTTI data
    Decl:string;
    Fixups:TList;
    Fields:TList;     //List of Fields
    Properties:TList; //List of Properties
    Methods:TList;    //List of Methods
  end;
  PTYPE_INFO = ^TYPE_INFO;

  PROCDECL_INFO = record
    ID:Integer;
    Offset:Cardinal;
    Size:Cardinal;
    Name:String;
    ModuleID:WORD;
    Embedded:Boolean;       //if true, contains embedded procs
    DumpType:Char;       //'C' - code, 'D' - data
    MethodKind:Char;     //'M'-method,'P'-procedure,'F'-function,'C'-constructor,'D'-destructor
    CallKind:TProcCallKind;
    VProc:Integer;
    DumpSz:Cardinal;         //Size of binary data
    DumpOfs:Cardinal;        //Offset of binary data
    TypeDef:string;
    Args:TList;
    ALocals:TList;
    Fixups:TList;
  end;
  PPROCDECL_INFO = ^PROCDECL_INFO;

  RESSTR_INFO = record
    ID:Integer;
    Offset:Cardinal;
    Size:Cardinal;
    ModuleID:WORD;
    Name:string;
    DumpOfs:Cardinal;        //Offset of binary data
    DumpSz:Cardinal;         //Size of binary data
    TypeDef:string;
    Context:string;          //Context of ResStr
  end;
  PRESSTR_INFO = ^RESSTR_INFO;

//#define VI_VAR          'V'
//#define VI_ABSVAR       'A'
//#define VI_SPECVAR      'S'
//#define VI_THREADVAR    'T'
  VAR_INFO = record
    ID:Integer;
    Offset:Cardinal;
    Size:Cardinal;
    ModuleID:WORD;
    Name:string;
    FType:Char;           //look above
    DumpOfs:Cardinal;     //Offset of binary data
    DumpSz:Cardinal;      //Size of binary data
    AbsName:string;
    TypeDef:string;
  end;
  PVAR_INFO = ^VAR_INFO;

  FIXUP_INFO = record
    FType:Char;           //'A'-ADR;'J'-JMP;'D'-DAT
    Ofs:Cardinal;         //Offset from RTTI data begin
    Name:String;
  end;
  PFIXUP_INFO = ^FIXUP_INFO;

  OFFSETSINFO = record
    Offset:Cardinal;
    Size:Cardinal;
    ModId:Integer;
    NamId:Integer;
  end;
  POFFSETSINFO = ^OFFSETSINFO;


var
  ActiveInfo, ActiveScope:TDeclSecKind;
  pDumpOffset:^Cardinal;
  pDumpSize:^Cardinal;
  pBlockOffset:Cardinal;
  VarOff:Boolean = False;
  FixupsList:TList = Nil;    //List of Fixups
  FieldsList:TList = Nil;    //List of Fields ("field")
  PropertiesList:TList = Nil;//List of Properties ("property")
  MethodsList:TList = Nil;   //List of Methods
  ArgsList:TList = Nil;      //("var","val")
  LocalsList:TList = Nil;    //List of local vars ("local","local absolute","result")
  KBUnitsList:string = '';
  FThreadVar:Boolean = False;
  ModuleList:TList = Nil;
  ModuleInfo:PMODULE_INFO = Nil;
  ModuleID:WORD;
  ConstList:TList = Nil;     //List of Constants
  TypeList:TList = Nil;      //List of Types
  VarList:TList = Nil;       //List of Vars
  ResStrList:TList = Nil;    //List of ResourceStrings
  ProcList:TList = Nil;      //List of Procedures
  ModuleCount:Integer = 0;
  MaxModuleDataSize:Integer = 0;
  ConstCount:Integer = 0;
  MaxConstDataSize:Integer = 0;
  TypeCount:Integer = 0;
  MaxTypeDataSize:Integer = 0;
  VarCount:Integer = 0;
  MaxVarDataSize:Integer = 0;
  ResStrCount:Integer = 0;
  MaxResStrDataSize:Integer = 0;
  ProcCount:Integer = 0;
  MaxProcDataSize:Integer = 0;
  KBStream:TFileStream = Nil;
  CurrOffset:Cardinal;
  CaseN:Integer = -1;
  KBSignature:string = 'IDR Knowledge Base File';
  KBIsMSIL:Boolean = False;
  KBFVer:Integer = 0;
  KBCRC:Cardinal = $FFFFFFFF;
  KBDescription:array[1..256] of Char;
  KBVersion:Integer = 2;
  KBCreateDT:TDateTime;
  KBLastModifyDT:TDateTime;

{Exports}
function CompareModulesByName(Item1:Pointer; Item2:Pointer):Integer;
function CompareModulesByID(Item1:Pointer; Item2:Pointer):Integer;
function CompareConstsByName(Item1:Pointer; Item2:Pointer):Integer;
function CompareConstsByID(Item1:Pointer; Item2:Pointer):Integer;
function CompareTypesByName(Item1:Pointer; Item2:Pointer):Integer;
function CompareTypesByID(Item1:Pointer; Item2:Pointer):Integer;
function CompareVarsByName(Item1:Pointer; Item2:Pointer):Integer;
function CompareVarsByID(Item1:Pointer; Item2:Pointer):Integer;
function CompareResStrsByName(Item1:Pointer; Item2:Pointer):Integer;
function CompareResStrsByID(Item1:Pointer; Item2:Pointer):Integer;
function CompareProcsByName(Item1:Pointer; Item2:Pointer):Integer;
function CompareProcsByID(Item1:Pointer; Item2:Pointer):Integer;
function WriteString(fDst:TFileStream; str:string):Integer;
function WriteDump(fSrc:TFileStream; fDst:TFileStream; SrcOffset:Cardinal; Bytes:Cardinal):Integer;
function WriteRelocs(fDst:TFileStream; FixupList:TList; Bytes:Cardinal; Name:String):Integer;
function WriteFixups(fDst:TFileStream; FixupList:TList):Integer;

procedure KBWriteModules(fDst:TFileStream);
procedure KBWriteConstants(fDst:TFileStream);
procedure KBWriteTypes(fDst:TFileStream);
procedure KBWriteVars(fDst:TFileStream);
procedure KBWriteResStrings(fDst:TFileStream);
procedure KBWriteProcedures(fDst:TFileStream);
procedure KBWriteOffsets(fDst:TFileStream);

implementation

function CompareModulesByName(Item1:Pointer; Item2:Pointer):Integer;
begin
  Result := CompareText(PMODULE_INFO(Item1)^.Name, PMODULE_INFO(Item2)^.Name);
end;

function CompareModulesByID(Item1:Pointer; Item2:Pointer):Integer;
begin
  if (PMODULE_INFO(Item1)^.ModuleID > PMODULE_INFO(Item2)^.ModuleID) then
    Result := 1
  else if (PMODULE_INFO(Item1)^.ModuleID < PMODULE_INFO(Item2)^.ModuleID) then
    Result := -1
  else
    Result := 0;
end;

function CompareConstsByName(Item1:Pointer; Item2:Pointer):Integer;
begin
  Result := CompareText(PCONST_INFO(Item1)^.Name, PCONST_INFO(Item2)^.Name);
  if (Result <> 0) then
    Exit;
  if (PCONST_INFO(Item1)^.ModuleID > PCONST_INFO(Item2)^.ModuleID) then
    Result := 1
  else if (PCONST_INFO(Item1)^.ModuleID < PCONST_INFO(Item2)^.ModuleID) then
    Result := -1
  else
    Result := 0;
end;

function CompareConstsByID(Item1:Pointer; Item2:Pointer):Integer;
begin
  if (PCONST_INFO(Item1)^.ModuleID > PCONST_INFO(Item2)^.ModuleID) then
    Result := 1
  else if (PCONST_INFO(Item1)^.ModuleID < PCONST_INFO(Item2)^.ModuleID) then
    Result := -1
  else
    Result := CompareText(PCONST_INFO(Item1)^.Name, PCONST_INFO(Item2)^.Name);
end;

function CompareTypesByName(Item1:Pointer; Item2:Pointer):Integer;
begin
  Result := CompareText(PTYPE_INFO(Item1)^.Name, PTYPE_INFO(Item2)^.Name);
  if (Result <> 0) then
    Exit;
  if (PTYPE_INFO(Item1)^.ModuleID > PTYPE_INFO(Item2)^.ModuleID) then
    Result := 1
  else if (PTYPE_INFO(Item1)^.ModuleID < PTYPE_INFO(Item2)^.ModuleID) then
    Result := -1
  else
    Result := 0;
end;

function CompareTypesByID(Item1:Pointer; Item2:Pointer):Integer;
begin
  if (PTYPE_INFO(Item1)^.ModuleID > PTYPE_INFO(Item2)^.ModuleID) then
    Result := 1
  else if (PTYPE_INFO(Item1)^.ModuleID < PTYPE_INFO(Item2)^.ModuleID) then
    Result := -1
  else
    Result := CompareText(PTYPE_INFO(Item1)^.Name, PTYPE_INFO(Item2)^.Name);
end;

function CompareVarsByName(Item1:Pointer; Item2:Pointer):Integer;
begin
  Result := CompareText(PVAR_INFO(Item1)^.Name, PVAR_INFO(Item2)^.Name);
  if (Result <> 0) then
    Exit;
  if (PVAR_INFO(Item1)^.ModuleID > PVAR_INFO(Item2)^.ModuleID) then
    Result := 1
  else if (PVAR_INFO(Item1)^.ModuleID < PVAR_INFO(Item2)^.ModuleID) then
    Result := -1
  else
    Result := 0;
end;

function CompareVarsByID(Item1:Pointer; Item2:Pointer):Integer;
begin
  if (PVAR_INFO(Item1)^.ModuleID > PVAR_INFO(Item2)^.ModuleID) then
    Result := 1
  else if (PVAR_INFO(Item1)^.ModuleID < PVAR_INFO(Item2)^.ModuleID) then
    Result := -1
  else
    Result := CompareText(PVAR_INFO(Item1)^.Name, PVAR_INFO(Item2)^.Name);
end;

function CompareResStrsByName(Item1:Pointer; Item2:Pointer):Integer;
begin
  Result := CompareText(PRESSTR_INFO(Item1)^.Name, PRESSTR_INFO(Item2)^.Name);
  if (Result <> 0) then
    Exit;
  if (PRESSTR_INFO(Item1)^.ModuleID > PRESSTR_INFO(Item2)^.ModuleID) then
    Result := 1
  else if (PRESSTR_INFO(Item1)^.ModuleID < PRESSTR_INFO(Item2)^.ModuleID) then
    Result := -1
  else
    Result := 0;
end;

function CompareResStrsByID(Item1:Pointer; Item2:Pointer):Integer;
begin
  if (PRESSTR_INFO(Item1)^.ModuleID > PRESSTR_INFO(Item2)^.ModuleID) then
    Result := 1
  else if (PRESSTR_INFO(Item1)^.ModuleID < PRESSTR_INFO(Item2)^.ModuleID) then
    Result := -1
  else
    Result := CompareText(PRESSTR_INFO(Item1)^.Name, PRESSTR_INFO(Item2)^.Name);
end;

function CompareProcsByName(Item1:Pointer; Item2:Pointer):Integer;
begin
  Result := CompareText(PPROCDECL_INFO(Item1)^.Name, PPROCDECL_INFO(Item2)^.Name);
  if (Result <> 0) then
    Exit;
  if (PPROCDECL_INFO(Item1)^.ModuleID > PPROCDECL_INFO(Item2)^.ModuleID) then
    Result := 1
  else if (PPROCDECL_INFO(Item1)^.ModuleID < PPROCDECL_INFO(Item2)^.ModuleID) then
    Result := -1
  else
    Result := 0;
end;

function CompareProcsByID(Item1:Pointer; Item2:Pointer):Integer;
begin
  if (PPROCDECL_INFO(Item1)^.ModuleID > PPROCDECL_INFO(Item2)^.ModuleID) then
    Result := 1
  else if (PPROCDECL_INFO(Item1)^.ModuleID < PPROCDECL_INFO(Item2)^.ModuleID) then
    Result := -1
  else
    Result := CompareText(PPROCDECL_INFO(Item1)^.Name, PPROCDECL_INFO(Item2)^.Name);
end;

function WriteString(fDst:TFileStream; str:string):Integer;
var
  Bytes:Integer;
  ZeroB:Byte;
  NameLength:WORD;
begin
  Bytes := 0;
  ZeroB := 0;

  NameLength := Length(str);
  if (fDst <> Nil) then
  begin
    fDst.Write(NameLength, sizeof(NameLength));
    if (NameLength > 0) then
    begin
        fDst.Write(Pointer(str)^, NameLength);
    end;
    fDst.Write(ZeroB, 1);
  end;
  Result := sizeof(NameLength) + NameLength + 1;
end;

function WriteDump(fSrc:TFileStream; fDst:TFileStream; SrcOffset:Cardinal; Bytes:Cardinal):Integer;
var
  b:BYTE;
  m:Integer;
begin
  fSrc.Seek(SrcOffset, soFromBeginning);
  for m := 1 to Bytes do
  begin
    fSrc.Read(b, 1);
    fDst.Write(b, 1);
  end;
  Result := Bytes;
end;

function WriteRelocs(fDst:TFileStream; FixupList:TList; Bytes:Cardinal; Name:String):Integer;
var
  n, m:Integer;
  Byte00:BYTE;
  ByteFF:Cardinal;
  ByteNo:Integer;
  finfo:PFIXUP_INFO;
begin
  Byte00 := 0;
  ByteFF := $FFFFFFFF;
  ByteNo := 0;
  for n := 0 to FixupList.Count - 1 do
  begin
    finfo := PFIXUP_INFO(FixupList.Items[n]);
    //If you need to inform that you have more to do with it, it’s not necessary
    if (finfo^.Ofs + 4 > Bytes) then
      continue;
    if (finfo^.Ofs < ByteNo) then
      continue;

    for m := ByteNo to finfo^.Ofs - 1 do
    begin
      fDst.Write(Byte00, 1);
      Inc(ByteNo);
    end;
    //Shoulder Accessories 4-part 0xFF
    fDst.Write(ByteFF, 4); Inc(ByteNo, 4);
  end;
  //Adjustable accessories 0
  for m := ByteNo to Bytes - 1 do
  begin
    fDst.Write(Byte00, 1);
  end;
  Result := Bytes;
end;

function WriteFixups(fDst:TFileStream; FixupList:TList):Integer;
var
  m:Integer;
  Bytes:Integer;
  finfo:PFIXUP_INFO;
begin
  Bytes := 0;
  for m := 0 to FixupList.Count -1 do
  begin
    finfo := PFIXUP_INFO(FixupList.Items[m]);
    //write Type
    if (fDst <> Nil) then
      fDst.Write(finfo^.FType, sizeof(finfo^.FType));
    Inc(Bytes, sizeof(finfo^.FType));
    //write Offset
    if (fDst <> Nil) then
      fDst.Write(finfo^.Ofs, sizeof(finfo^.Ofs));
    Inc(Bytes, sizeof(finfo^.Ofs));
    //write Name
    Inc(Bytes, WriteString(fDst, finfo^.Name));
  end;
  Result := Bytes;
end;

procedure KBWriteModules(fDst:TFileStream);
var
  found:Boolean;
  mID:WORD;
  n, m, mm:Integer;
  UsesNum:WORD;
  DataSize:Cardinal;
  ModuleInfo, modInfo:PMODULE_INFO;
begin
  ModuleCount := ModuleList.Count;
  for n := 0 to ModuleCount - 1 do
  begin
    ModuleInfo := PMODULE_INFO(ModuleList.Items[n]);
    ModuleInfo^.ID := n;
    ModuleInfo^.Offset := CurrOffset;
    DataSize := 0;
    //ModuleID
    fDst.Write(ModuleInfo^.ModuleID, sizeof(WORD));
    Inc(DataSize, sizeof(WORD));
    //Name
    Inc(DataSize, WriteString(fDst, ModuleInfo^.Name));
    //Filename
    Inc(DataSize, WriteString(fDst, ModuleInfo^.Filename));
    //UsesNum
    UsesNum := ModuleInfo.UsesList.Count;
    fDst.Write(UsesNum, sizeof(UsesNum));
    Inc(DataSize, sizeof(UsesNum));
    //Uses
    for m := 0 to UsesNum - 1 do
    begin
      found := False;
      for mm := 0 to ModuleCount - 1 do
      begin
        modInfo := PMODULE_INFO(ModuleList.Items[mm]);
        if SameText(modInfo.Name, ModuleInfo.UsesList.Strings[m]) then
        begin
          fDst.Write(modInfo^.ModuleID, sizeof(WORD));
          found := True;
          Break;
        end;
      end;
      if (found = False) then
      begin
        mID := $FFFF;
        fDst.Write(mID, sizeof(WORD));
      end;
      Inc(DataSize, sizeof(WORD));
    end;
    for m := 0 to UsesNum - 1 do
    begin
      Inc(DataSize, WriteString(fDst, ModuleInfo.UsesList.Strings[m]));
    end;

    ModuleInfo.Size := DataSize;
    if (DataSize > MaxModuleDataSize) then
    begin
      MaxModuleDataSize := DataSize;
    end;
    Inc(CurrOffset, DataSize);
  end;
end;

procedure KBWriteConstants(fDst:TFileStream);
var
  fIn:TFileStream;
  n, m:Integer;
  PrevModId:WORD;
  DataSize:Cardinal;
  DumpTotal:Cardinal;
  FixupNum:Cardinal;
  PrevName:string;
  constInfo: PCONST_INFO;
begin
  fIn := Nil;
  ConstList.Sort(CompareConstsByName);
  PrevModId := $FFFF;
  PrevName := '';

  for n := 0 to ConstList.Count - 1 do
  begin
    constInfo := PCONST_INFO(ConstList.Items[n]);
    constInfo^.Skip := False;
    if (SameText(constInfo^.Name, PrevName)) And (constInfo^.ModuleID = PrevModId) then
    begin
      constInfo^.Skip := True;
      continue;
    end;
    constInfo^.ID := ConstCount;
    constInfo^.Offset := CurrOffset;
    PrevModId := constInfo^.ModuleID;
    PrevName := constInfo^.Name;

    DataSize := 0;
    //ModuleID
    fDst.Write(constInfo^.ModuleID, sizeof(constInfo^.ModuleID));
    Inc(DataSize, sizeof(constInfo^.ModuleID));
    //Name
    Inc(DataSize, WriteString(fDst, constInfo^.Name));
    //Type
    fDst.Write(constInfo^.FType, sizeof(constInfo^.FType));
    Inc(DataSize, sizeof(constInfo^.FType));
    //TypeDef
    Inc(DataSize, WriteString(fDst, constInfo^.TypeDef));
    //Value
    Inc(DataSize, WriteString(fDst, constInfo^.Value));
    //For more information on the use of the vehicle, please contact us.
    if (Pos('_NV_', constInfo^.Name) = 1) then
      constInfo^.RTTISz := 0;
    //DumpTotal
    DumpTotal := 0;
    if (constInfo^.RTTISz <> 0) then
    begin
      fIn := Nil;
      for m := 0 to ModuleCount - 1 do
      begin
        ModuleInfo := PMODULE_INFO(ModuleList.Items[m]);
        if (ModuleInfo^.ModuleID = constInfo^.ModuleID) then
        begin
          fIn := TFileStream.Create(ModuleInfo^.Filename, fmOpenRead);
          break;
        end;
      end;
      if (fIn <> Nil) then
      begin
        //Dump
        Inc(DumpTotal, constInfo^.RTTISz);
        //Relocs
        Inc(DumpTotal, constInfo^.RTTISz);
        //Fixups
        Inc(DumpTotal, WriteFixups(Nil, constInfo^.Fixups));
      end;
    end;
    Inc(DumpTotal, sizeof(constInfo^.RTTISz)); //DumpSz
    FixupNum := constInfo^.Fixups.Count;
    Inc(DumpTotal, sizeof(FixupNum));          //FixupNum

    fDst.Write(DumpTotal, sizeof(DumpTotal));
    Inc(DataSize, sizeof(DumpTotal));
    //DumpSz
    fDst.Write(constInfo^.RTTISz, sizeof(constInfo^.RTTISz));
    Inc(DataSize, sizeof(constInfo^.RTTISz));
    //FixupNum
    fDst.Write(FixupNum, sizeof(FixupNum));
    Inc(DataSize, sizeof(FixupNum));

    if (constInfo^.RTTISz <> 0) then
    begin
      if (fIn <> Nil) then
      begin
        //Dump
        Inc(DataSize, WriteDump(fIn, fDst, constInfo^.RTTIOfs, constInfo^.RTTISz));
        fIn.Free;
        //Relocs
        Inc(DataSize, WriteRelocs(fDst, constInfo^.Fixups, constInfo^.RTTISz, constInfo^.Name));
        //Fixups
        Inc(DataSize, WriteFixups(fDst, constInfo^.Fixups));
      end;
    end;

    constInfo^.Size := DataSize;
    if (DataSize > MaxConstDataSize) then
      MaxConstDataSize := DataSize;
    Inc(CurrOffset, DataSize);
    Inc(ConstCount);
  end;
end;

procedure KBWriteTypes(fDst:TFileStream);
var
  FieldsNum:WORD;
  PropsNum:WORD;
  MethodsNum:WORD;
  fIn:TFileStream;
  n, m:Integer;
  DataSize:Cardinal;
  DumpTotal:Cardinal;
  PropsTotal:Cardinal;
  MethodsTotal:Cardinal;
  FixupNum:Cardinal;
  FieldsTotal:Cardinal;
  typeInfo:PTYPE_INFO;
  linfo:PLOCALDECL_INFO;
  pinfo:PPROPERTY_INFO;
  minfo:PMETHODDECL_INFO;
begin
  fIn := Nil;
  TypeCount := TypeList.Count;
  for n := 0 to TypeCount -1 do
  begin
    typeInfo := PTYPE_INFO(TypeList.Items[n]);
    typeInfo^.ID := n;
    typeInfo^.Offset := CurrOffset;
    DataSize := 0;
    //Size (NEW_VERSION)
    fDst.Write(typeInfo^.Size, sizeof(typeInfo^.Size));
    Inc(DataSize, sizeof(typeInfo^.Size));
    //ModuleID
    fDst.Write(typeInfo^.ModuleID, sizeof(typeInfo^.ModuleID));
    Inc(DataSize, sizeof(typeInfo^.ModuleID));
    //Name
    Inc(DataSize, WriteString(fDst, typeInfo^.Name));
    //Kind
    fDst.Write(typeInfo^.Kind, sizeof(typeInfo^.Kind));
    Inc(DataSize, sizeof(typeInfo^.Kind));
    //VMCnt
    fDst.Write(typeInfo^.VMCnt, sizeof(typeInfo^.VMCnt));
    Inc(DataSize, sizeof(typeInfo^.VMCnt));
    //Decl
    Inc(DataSize, WriteString(fDst, typeInfo^.Decl));
    //If you want to change the type of equipment
    if (Pos('_NT_', typeInfo^.Name) = 1) then
      typeInfo^.RTTISz := 0;
    //DumpTotal
    DumpTotal := 0;
    if (typeInfo^.RTTISz <> 0) then
    begin
      fIn := Nil;
      for m := 0 to ModuleList.Count - 1 do
      begin
        ModuleInfo := PMODULE_INFO(ModuleList.Items[m]);
        if (ModuleInfo^.ModuleID = typeInfo^.ModuleID) then
        begin
          fIn := TFileStream.Create(ModuleInfo^.Filename, fmOpenRead);
          break;
        end;
      end;
      if (fIn <> Nil) then
      begin
        //Dump
        Inc(DumpTotal, typeInfo^.RTTISz);
        //Relocs
        Inc(DumpTotal, typeInfo^.RTTISz);
        //Fixups
        Inc(DumpTotal, WriteFixups(Nil, typeInfo^.Fixups));
      end;
    end;
    Inc(DumpTotal, sizeof(typeInfo^.RTTISz));  //DumpSz
    FixupNum := typeInfo^.Fixups.Count;
    Inc(DumpTotal, sizeof(FixupNum));          //FixupNum

    fDst.Write(DumpTotal, sizeof(DumpTotal));
    Inc(DataSize, sizeof(DumpTotal));
    //DumpSz
    fDst.Write(typeInfo^.RTTISz, sizeof(typeInfo^.RTTISz));
    Inc(DataSize, sizeof(typeInfo^.RTTISz));
    //FixupNum
    fDst.Write(FixupNum, sizeof(FixupNum));
    Inc(DataSize, sizeof(FixupNum));

    if (typeInfo^.RTTISz <> 0) then
    begin
      if (fIn <> Nil) then
      begin
        //Dump
        Inc(DataSize, WriteDump(fIn, fDst, typeInfo^.RTTIOfs, typeInfo^.RTTISz));
        fIn.Free;
        //Relocs
        Inc(DataSize, WriteRelocs(fDst, typeInfo^.Fixups, typeInfo^.RTTISz, typeInfo^.Name));
        //Fixups
        Inc(DataSize, WriteFixups(fDst, typeInfo^.Fixups));
      end;
    end;
    //FieldsTotal
    FieldsNum := typeInfo^.Fields.Count;
    FieldsTotal := 0;

    for m := 0 to FieldsNum -1 do
    begin
      linfo := PLOCALDECL_INFO(typeInfo^.Fields.Items[m]);
      Inc(FieldsTotal, sizeof(linfo^.Scope));
      Inc(FieldsTotal, sizeof(linfo^.Ndx));
      Inc(FieldsTotal, sizeof(linfo^.FCase));
      Inc(FieldsTotal, WriteString(Nil, linfo^.Name));
      Inc(FieldsTotal, WriteString(Nil, linfo^.TypeDef));
    end;
    Inc(FieldsTotal, sizeof(FieldsNum));   //FieldsNum

    fDst.Write(FieldsTotal, sizeof(FieldsTotal));
    Inc(DataSize, sizeof(FieldsTotal));
    //FieldsNum
    fDst.Write(FieldsNum, sizeof(FieldsNum));
    Inc(DataSize, sizeof(FieldsNum));
    //Fields
    for m := 0 to FieldsNum -1 do
    begin
      linfo := PLOCALDECL_INFO(typeInfo^.Fields.Items[m]);
      fDst.Write(linfo^.Scope, sizeof(linfo^.Scope));
      Inc(DataSize, sizeof(linfo^.Scope));
      fDst.Write(linfo^.Ndx, sizeof(linfo^.Ndx));
      Inc(DataSize, sizeof(linfo^.Ndx));
      fDst.Write(linfo^.FCase, sizeof(linfo^.FCase));
      Inc(DataSize, sizeof(linfo^.FCase));
      Inc(DataSize, WriteString(fDst, linfo^.Name));
      Inc(DataSize, WriteString(fDst, linfo^.TypeDef));
    end;
    //PropsTotal
    PropsNum := typeInfo^.Properties.Count;
    PropsTotal := 0;
    for m := 0 to PropsNum -1 do
    begin
      if (typeInfo^.Kind = drClassDef) Or (typeInfo^.Kind = drInterfaceDef) then
      begin
        pinfo := PPROPERTY_INFO(typeInfo^.Properties.Items[m]);
        Inc(PropsTotal, sizeof(pinfo^.Scope));
        Inc(PropsTotal, sizeof(pinfo^.Index));
        Inc(PropsTotal, sizeof(pinfo^.FDispId));
        Inc(PropsTotal, WriteString(Nil, pinfo^.Name));
        Inc(PropsTotal, WriteString(Nil, pinfo^.TypeDef));
        Inc(PropsTotal, WriteString(Nil, pinfo^.ReadName));
        Inc(PropsTotal, WriteString(Nil, pinfo^.WriteName));
        Inc(PropsTotal, WriteString(Nil, pinfo^.StoredName));
      end;
    end;
    Inc(PropsTotal, sizeof(PropsNum)); //PropsNum

    fDst.Write(PropsTotal, sizeof(PropsTotal));
    Inc(DataSize, sizeof(PropsTotal));
    //PropsNum
    fDst.Write(PropsNum, sizeof(PropsNum));
    Inc(DataSize, sizeof(PropsNum));
    //Props
    for m := 0 to PropsNum -1 do
    begin
      if (typeInfo^.Kind = drClassDef) Or (typeInfo^.Kind = drInterfaceDef) then
      begin
        pinfo := PPROPERTY_INFO(typeInfo^.Properties.Items[m]);
        fDst.Write(pinfo^.Scope, sizeof(pinfo^.Scope));
        Inc(DataSize, sizeof(pinfo^.Scope));
        fDst.Write(pinfo^.Index, sizeof(pinfo^.Index));
        Inc(DataSize, sizeof(pinfo^.Index));
        fDst.Write(pinfo^.FDispId, sizeof(pinfo^.FDispId));
        Inc(DataSize, sizeof(pinfo^.FDispId));
        Inc(DataSize, WriteString(fDst, pinfo^.Name));
        Inc(DataSize, WriteString(fDst, pinfo^.TypeDef));
        Inc(DataSize, WriteString(fDst, pinfo^.ReadName));
        Inc(DataSize, WriteString(fDst, pinfo^.WriteName));
        Inc(DataSize, WriteString(fDst, pinfo^.StoredName));
      end;
    end;
    //MethodsTotal
    MethodsNum := typeInfo^.Methods.Count;
    MethodsTotal := 0;
    for m := 0 to MethodsNum - 1 do
    begin
        minfo := PMETHODDECL_INFO(typeInfo^.Methods.Items[m]);
        Inc(MethodsTotal, sizeof(minfo^.Scope));
        Inc(MethodsTotal, sizeof(minfo^.MethodKind));
        Inc(MethodsTotal, WriteString(Nil, minfo^.Prototype));
    end;
    Inc(MethodsTotal, sizeof(MethodsNum)); //MethodsNum

    fDst.Write(MethodsTotal, sizeof(MethodsTotal));
    Inc(DataSize, sizeof(MethodsTotal));
    //MethodsNum
    fDst.Write(MethodsNum, sizeof(MethodsNum));
    Inc(DataSize, sizeof(MethodsNum));
    //Methods
    for m := 0 to MethodsNum -1 do
    begin
        minfo := PMETHODDECL_INFO(typeInfo^.Methods.Items[m]);
        fDst.Write(minfo^.Scope, sizeof(minfo^.Scope));
        Inc(DataSize, sizeof(minfo^.Scope));
        fDst.Write(minfo^.MethodKind, sizeof(minfo^.MethodKind));
        Inc(DataSize, sizeof(minfo^.MethodKind));
        Inc(DataSize, WriteString(fDst, minfo^.Prototype));
    end;
    typeInfo^.Size := DataSize;
    if (DataSize > MaxTypeDataSize) then
      MaxTypeDataSize := DataSize;
    Inc(CurrOffset, DataSize);
  end;
end;

procedure KBWriteVars(fDst:TFileStream);
var
  n:Integer;
  DataSize:Cardinal;
  vInfo:PVAR_INFO;
begin
  VarCount := VarList.Count;
  for n := 0 to VarCount -1 do
  begin
    vInfo := PVAR_INFO(VarList.Items[n]);
    vInfo^.ID := n;
    vInfo^.Offset := CurrOffset;

    DataSize := 0;
    //ModuleID
    fDst.Write(vInfo^.ModuleID, sizeof(vInfo^.ModuleID));
    Inc(DataSize, sizeof(vInfo^.ModuleID));
    //Name
    Inc(DataSize, WriteString(fDst, vInfo^.Name));
    //Type
    fDst.Write(vInfo^.FType, sizeof(vInfo^.FType));
    Inc(DataSize, sizeof(vInfo^.FType));
    //TypeDef
    Inc(DataSize, WriteString(fDst, vInfo^.TypeDef));
    //AbsName
    Inc(DataSize, WriteString(fDst, vInfo^.AbsName));

    vInfo^.Size := DataSize;
    if (DataSize > MaxVarDataSize) then
      MaxVarDataSize := DataSize;
    Inc(CurrOffset, DataSize);
  end;
end;

procedure KBWriteResStrings(fDst:TFileStream);
var
  ResStrLen:WORD;
  wptr:^WORD;
  cptr:PChar;
  n, m:Integer;
  DataSize:Cardinal;
  fIn:TFileStream;
  rsInfo:PRESSTR_INFO;
  ResStrBuf:array[0..4096] of BYTE;
begin
  ResStrCount := ResStrList.Count;
  for n := 0 to ResStrCount - 1 do
  begin
    rsInfo := PRESSTR_INFO(ResStrList.Items[n]);
    rsInfo^.ID := n;
    rsInfo^.Offset := CurrOffset;

    DataSize := 0;
    //ModuleID
    fDst.Write(rsInfo^.ModuleID, sizeof(rsInfo^.ModuleID));
    Inc(DataSize, sizeof(rsInfo^.ModuleID));
    //Name
    Inc(DataSize, WriteString(fDst, rsInfo^.Name));
    //TypeDef
    Inc(DataSize, WriteString(fDst, rsInfo^.TypeDef));
    //Context
    Inc(DataSize, WriteString(fDst, rsInfo^.Context));
    {*
    if (rsInfo^.AContext <> '') then
      Inc(DataSize, WriteString(fDst, rsInfo^.AContext))
    else if (rsInfo^.DumpSz <> 0) then
    begin
        fIn := Nil;
        for m := 0 to ModuleList.Count - 1 do
        begin
            ModuleInfo := PMODULE_INFO(ModuleList.Items[m]);
            if (ModuleInfo^.ModuleID = rsInfo^.ModuleID) then
            begin
                fIn := TFileStream.Create(ModuleInfo^.Filename, fmOpenRead);
                break;
            end;
        end;
        if (fIn <> Nil) then
        begin
            //Context
            fIn.Seek(rsInfo^.DumpOfs, soFromBeginning);
            fIn.Read(ResStrBuf, rsInfo^.DumpSz);
            fIn.Free;
            wptr := @ResStrBuf[4];
            ResStrLen := wptr^;
            cptr := @ResStrBuf[8];
            rsInfo^.AContext := Copy(cptr, 1, ResStrLen);
            Inc(DataSize, WriteString(fDst, rsInfo^.AContext));
        end;
    end;
    *}
    rsInfo^.Size := DataSize;
    if (DataSize > MaxResStrDataSize) then
      MaxResStrDataSize := DataSize;
    Inc(CurrOffset, DataSize);
  end;
end;

procedure KBWriteProcedures(fDst:TFileStream);
var
  n, m:Integer;
  ArgsNum, LocalsNum:WORD;
  DataSize, DumpTotal, FixupNum, ArgsTotal, LocalsTotal:Cardinal;
  fIn:TFileStream;
  pInfo:PPROCDECL_INFO;
  lInfo:PLOCALDECL_INFO;
begin
  fIn := Nil;
  ProcCount := ProcList.Count;
  for n := 0 to ProcCount - 1 do
  begin
    DataSize := 0;
    pInfo := PPROCDECL_INFO(ProcList.Items[n]);
    pInfo^.ID := n;
    pInfo^.Offset := CurrOffset;
    //ModuleID
    fDst.Write(pInfo^.ModuleID, sizeof(pInfo^.ModuleID));
    Inc(DataSize, sizeof(pInfo^.ModuleID));
    //Name
    Inc(DataSize, WriteString(fDst, pInfo^.Name));
    //Embedded
    fDst.Write(pInfo^.Embedded, sizeof(pInfo^.Embedded));
    Inc(DataSize, sizeof(pInfo^.Embedded));
    //DumpType
    fDst.Write(pInfo^.DumpType, sizeof(pInfo^.DumpType));
    Inc(DataSize, sizeof(pInfo^.DumpType));
    //MethodKind
    fDst.Write(pInfo^.MethodKind, sizeof(pInfo^.MethodKind));
    Inc(DataSize, sizeof(pInfo^.MethodKind));
    //CallKind
    fDst.Write(pInfo^.CallKind, sizeof(pInfo^.CallKind));
    Inc(DataSize, sizeof(pInfo^.CallKind));
    //VProc
    fDst.Write(pInfo^.VProc, sizeof(pInfo^.VProc));
    Inc(DataSize, sizeof(pInfo^.VProc));
    //TypeDef
    Inc(DataSize, WriteString(fDst, pInfo^.TypeDef));
    //DumpTotal
    DumpTotal := 0;
    if (pInfo^.DumpSz <> 0) then
    begin
      fIn := Nil;
      for m := 0 to ModuleList.Count - 1 do
      begin
        ModuleInfo := PMODULE_INFO(ModuleList.Items[m]);
        if (ModuleInfo^.ModuleID = pInfo^.ModuleID) then
        begin
          fIn := TFileStream.Create(ModuleInfo^.Filename, fmOpenRead);
          break;
        end;
      end;
      if (fIn <> Nil) then
      begin
        //Dump
        Inc(DumpTotal, pInfo^.DumpSz);
        //Relocs
        Inc(DumpTotal, pInfo^.DumpSz);
        //Fixups
        Inc(DumpTotal, WriteFixups(Nil, pInfo^.Fixups));
      end;
    end;
    Inc(DumpTotal, sizeof(pInfo^.DumpSz)); //DumpSz
    FixupNum := pInfo^.Fixups.Count;
    Inc(DumpTotal, sizeof(FixupNum));      //FixupNum

    fDst.Write(DumpTotal, sizeof(DumpTotal));
    Inc(DataSize, sizeof(DumpTotal));
    //DumpSz
    fDst.Write(pInfo^.DumpSz, sizeof(pInfo^.DumpSz));
    Inc(DataSize, sizeof(pInfo^.DumpSz));
    //FixupNum
    fDst.Write(FixupNum, sizeof(FixupNum));
    Inc(DataSize, sizeof(FixupNum));

    if (pInfo^.DumpSz <> 0) then
    begin
      if (fIn <> Nil) then
      begin
        //Dump
        Inc(DataSize, WriteDump(fIn, fDst, pInfo^.DumpOfs, pInfo^.DumpSz));
        fIn.Free;
        //Relocs
        Inc(DataSize, WriteRelocs(fDst, pInfo^.Fixups, pInfo^.DumpSz, pInfo^.Name));
        //Fixups
        Inc(DataSize, WriteFixups(fDst, pInfo^.Fixups));
      end;
    end;

    //ArgsTotal
    ArgsNum := pInfo^.Args.Count;
    ArgsTotal := 0;
    for m := 0 to ArgsNum - 1 do
    begin
      lInfo := PLOCALDECL_INFO(pInfo^.Args.Items[m]);
      Inc(ArgsTotal, sizeof(lInfo^.Tag));
      Inc(ArgsTotal, sizeof(lInfo^.LocFlags));
      Inc(ArgsTotal, sizeof(lInfo^.Ndx));
      Inc(ArgsTotal, WriteString(Nil, lInfo^.Name));
      Inc(ArgsTotal, WriteString(Nil, lInfo^.TypeDef));
    end;
    Inc(ArgsTotal, sizeof(ArgsNum));

    fDst.Write(ArgsTotal, sizeof(ArgsTotal));
    Inc(DataSize, sizeof(ArgsTotal));
    //ArgsNum
    fDst.Write(ArgsNum, sizeof(ArgsNum));
    Inc(DataSize, sizeof(ArgsNum));
    //Args
    for m := 0 to ArgsNum - 1 do
    begin
      lInfo := PLOCALDECL_INFO(pInfo^.Args.Items[m]);

      fDst.Write(lInfo^.Tag, sizeof(lInfo^.Tag));
      Inc(DataSize, sizeof(lInfo^.Tag));
      fDst.Write(lInfo^.LocFlags, sizeof(lInfo^.LocFlags));
      Inc(DataSize, sizeof(lInfo^.LocFlags));
      fDst.Write(lInfo^.Ndx, sizeof(lInfo^.Ndx));
      Inc(DataSize, sizeof(lInfo^.Ndx));
      Inc(DataSize, WriteString(fDst, lInfo^.Name));
      Inc(DataSize, WriteString(fDst, lInfo^.TypeDef));
    end;
    //LocalsTotal
    {*
    LocalsNum := pInfo^.Locals.Count;
    LocalsTotal := 0;
    for m := 0 to LocalsNum - 1 do
    begin
      lInfo := PLOCALDECL_INFO(pInfo^.Locals.Items[m]);
      Inc(LocalsTotal, sizeof(lInfo^.Tag));
      Inc(LocalsTotal, sizeof(lInfo^.LocFlags));
      Inc(LocalsTotal, sizeof(lInfo^.Ndx));
      Inc(LocalsTotal, WriteString(Nil, lInfo^.Name));
      Inc(LocalsTotal, WriteString(Nil, lInfo^.TypeDef));
      Inc(LocalsTotal, WriteString(Nil, lInfo^.AbsName));
    end;
    Inc(LocalsTotal, sizeof(LocalsNum));
    fDst.Write(LocalsTotal, sizeof(LocalsTotal));
    Inc(DataSize, sizeof(LocalsTotal));
    //LocalsNum
    fDst.Write(LocalsNum, sizeof(LocalsNum));
    Inc(DataSize, sizeof(LocalsNum));
    //Locals
    for m := 0 to LocalsNum - 1 do
    begin
      lInfo := PLOCALDECL_INFO(pInfo^.Locals.Items[m]);
      fDst.Write(lInfo^.Tag, sizeof(lInfo^.Tag));
      Inc(DataSize, sizeof(lInfo^.Tag));
      fDst.Write(lInfo^.LocFlags, sizeof(lInfo^.LocFlags));
      Inc(DataSize, sizeof(lInfo^.LocFlags));
      fDst.Write(lInfo^.Ndx, sizeof(lInfo^.Ndx));
      Inc(DataSize, sizeof(lInfo^.Ndx));
      Inc(DataSize, WriteString(fDst, lInfo^.Name));
      Inc(DataSize, WriteString(fDst, lInfo^.TypeDef));
      Inc(DataSize, WriteString(fDst, lInfo^.AbsName));
    end;
    *}
    pInfo^.Size := DataSize;
    if (DataSize > MaxProcDataSize) then MaxProcDataSize := DataSize;
    Inc(CurrOffset, DataSize);
  end;
end;

procedure KBWriteOffsets(fDst:TFileStream);
var
  n, cn:Integer;
  ConstInfo:PCONST_INFO;
  TypeInfo:PTYPE_INFO;
  VarInfo:PVAR_INFO;
  ResInfo:PRESSTR_INFO;
  ProcInfo:PPROCDECL_INFO;
  Offsets:array of OFFSETSINFO;
begin
  //Modules
  fDst.Write(ModuleCount, sizeof(ModuleCount));
  fDst.Write(MaxModuleDataSize, sizeof(MaxModuleDataSize));
  SetLength(Offsets, ModuleCount);
  for n := 0 to ModuleCount - 1 do
  begin
    ModuleInfo := PMODULE_INFO(ModuleList.Items[n]);
    Offsets[n].Offset := ModuleInfo^.Offset;
    Offsets[n].Size := ModuleInfo^.Size;
  end;
  ModuleList.Sort(CompareModulesByID);
  for n := 0 to ModuleCount - 1 do
  begin
    ModuleInfo := PMODULE_INFO(ModuleList.Items[n]);
    Offsets[n].ModId := ModuleInfo^.ID;
  end;
  ModuleList.Sort(CompareModulesByName);
  for n := 0 to ModuleCount - 1 do
  begin
    ModuleInfo := PMODULE_INFO(ModuleList.Items[n]);
    Offsets[n].NamId := ModuleInfo^.ID;
  end;
  for n := 0 to ModuleCount - 1 do
  begin
    fDst.Write(Offsets[n].Offset, sizeof(Offsets[n].Offset));
    fDst.Write(Offsets[n].Size, sizeof(Offsets[n].Size));
    fDst.Write(Offsets[n].ModId, sizeof(Offsets[n].ModId));
    fDst.Write(Offsets[n].NamId, sizeof(Offsets[n].NamId));
  end;
  Offsets := nil;
  //Consts
  fDst.Write(ConstCount, sizeof(ConstCount));
  fDst.Write(MaxConstDataSize, sizeof(MaxConstDataSize));
  SetLength(Offsets, ConstCount);
  cn := 0;
  for n := 0 to ConstList.Count - 1 do
  begin
    ConstInfo := PCONST_INFO(ConstList.Items[n]);
    if (constInfo.Skip) then continue;

    Offsets[cn].Offset := ConstInfo^.Offset;
    Offsets[cn].Size := ConstInfo^.Size;
    Inc(cn);
  end;
  ConstList.Sort(CompareConstsByID);
  cn := 0;
  for n := 0 to ConstList.Count - 1 do
  begin
    ConstInfo := PCONST_INFO(ConstList.Items[n]);
    if (constInfo.Skip) then continue;

    Offsets[cn].ModId := ConstInfo^.ID;
    Inc(cn);
  end;
  ConstList.Sort(CompareConstsByName);
  cn := 0;
  for n := 0 to ConstList.Count - 1 do
  begin
      ConstInfo := PCONST_INFO(ConstList.Items[n]);
      if (constInfo.Skip) then continue;

      Offsets[cn].NamId := ConstInfo^.ID;
      Inc(cn);
  end;
  for n := 0 to ConstCount - 1 do
  begin
    fDst.Write(Offsets[n].Offset, sizeof(Offsets[n].Offset));
    fDst.Write(Offsets[n].Size, sizeof(Offsets[n].Size));
    fDst.Write(Offsets[n].ModId, sizeof(Offsets[n].ModId));
    fDst.Write(Offsets[n].NamId, sizeof(Offsets[n].NamId));
  end;
  Offsets := nil;
  //Types
  fDst.Write(TypeCount, sizeof(TypeCount));
  fDst.Write(MaxTypeDataSize, sizeof(MaxTypeDataSize));
  SetLength(Offsets, TypeCount);
  for n := 0 to TypeCount - 1 do
  begin
      TypeInfo := PTYPE_INFO(TypeList.Items[n]);
      Offsets[n].Offset := TypeInfo^.Offset;
      Offsets[n].Size := TypeInfo^.Size;
  end;
  TypeList.Sort(CompareTypesByID);
  for n := 0 to TypeCount - 1 do
  begin
      TypeInfo := PTYPE_INFO(TypeList.Items[n]);
      Offsets[n].ModId := TypeInfo^.ID;
  end;
  TypeList.Sort(CompareTypesByName);
  for n := 0 to TypeCount - 1 do
  begin
      TypeInfo := PTYPE_INFO(TypeList.Items[n]);
      Offsets[n].NamId := TypeInfo^.ID;
  end;
  for n := 0 to TypeCount - 1 do
  begin
    fDst.Write(Offsets[n].Offset, sizeof(Offsets[n].Offset));
    fDst.Write(Offsets[n].Size, sizeof(Offsets[n].Size));
    fDst.Write(Offsets[n].ModId, sizeof(Offsets[n].ModId));
    fDst.Write(Offsets[n].NamId, sizeof(Offsets[n].NamId));
  end;
  Offsets := nil;
  //Vars
  fDst.Write(VarCount, sizeof(VarCount));
  fDst.Write(MaxVarDataSize, sizeof(MaxVarDataSize));
  SetLength(Offsets, VarCount);
  for n := 0 to VarCount - 1 do
  begin
      VarInfo := PVAR_INFO(VarList.Items[n]);
      Offsets[n].Offset := VarInfo^.Offset;
      Offsets[n].Size := VarInfo^.Size;
  end;
  VarList.Sort(CompareVarsByID);
  for n := 0 to VarCount - 1 do
  begin
      VarInfo := PVAR_INFO(VarList.Items[n]);
      Offsets[n].ModId := VarInfo^.ID;
  end;
  VarList.Sort(CompareVarsByName);
  for n := 0 to VarCount - 1 do
  begin
      VarInfo := PVAR_INFO(VarList.Items[n]);
      Offsets[n].NamId := VarInfo^.ID;
  end;
  for n := 0 to VarCount - 1 do
  begin
    fDst.Write(Offsets[n].Offset, sizeof(Offsets[n].Offset));
    fDst.Write(Offsets[n].Size, sizeof(Offsets[n].Size));
    fDst.Write(Offsets[n].ModId, sizeof(Offsets[n].ModId));
    fDst.Write(Offsets[n].NamId, sizeof(Offsets[n].NamId));
  end;
  Offsets := nil;
  //ResStrings
  fDst.Write(ResStrCount, sizeof(ResStrCount));
  fDst.Write(MaxResStrDataSize, sizeof(MaxResStrDataSize));
  SetLength(Offsets, ResStrCount);
  for n := 0 to ResStrCount - 1 do
  begin
      ResInfo := PRESSTR_INFO(ResStrList.Items[n]);
      Offsets[n].Offset := ResInfo^.Offset;
      Offsets[n].Size := ResInfo^.Size;
  end;
  ResStrList.Sort(CompareResStrsByID);
  for n := 0 to ResStrCount - 1 do
  begin
      ResInfo := PRESSTR_INFO(ResStrList.Items[n]);
      Offsets[n].ModId := ResInfo^.ID;
  end;
  ResStrList.Sort(CompareResStrsByName);
  for n := 0 to ResStrCount - 1 do
  begin
      ResInfo := PRESSTR_INFO(ResStrList.Items[n]);
      Offsets[n].NamId := ResInfo^.ID;
  end;
  for n := 0 to ResStrCount - 1 do
  begin
    fDst.Write(Offsets[n].Offset, sizeof(Offsets[n].Offset));
    fDst.Write(Offsets[n].Size, sizeof(Offsets[n].Size));
    fDst.Write(Offsets[n].ModId, sizeof(Offsets[n].ModId));
    fDst.Write(Offsets[n].NamId, sizeof(Offsets[n].NamId));
  end;
  Offsets := nil;
  //Procs
  fDst.Write(ProcCount, sizeof(ProcCount));
  fDst.Write(MaxProcDataSize, sizeof(MaxProcDataSize));
  SetLength(Offsets, ProcCount);
  for n := 0 to ProcCount - 1 do
  begin
      ProcInfo := PPROCDECL_INFO(ProcList.Items[n]);
      Offsets[n].Offset := ProcInfo^.Offset;
      Offsets[n].Size := ProcInfo^.Size;
  end;
  ProcList.Sort(CompareProcsByID);
  for n := 0 to ProcCount - 1 do
  begin
      ProcInfo := PPROCDECL_INFO(ProcList.Items[n]);
      Offsets[n].ModId := ProcInfo^.ID;
  end;
  ProcList.Sort(CompareProcsByName);
  for n := 0 to ProcCount - 1 do
  begin
      ProcInfo := PPROCDECL_INFO(ProcList.Items[n]);
      Offsets[n].NamId := ProcInfo^.ID;
  end;
  for n := 0 to ProcCount - 1 do
  begin
    fDst.Write(Offsets[n].Offset, sizeof(Offsets[n].Offset));
    fDst.Write(Offsets[n].Size, sizeof(Offsets[n].Size));
    fDst.Write(Offsets[n].ModId, sizeof(Offsets[n].ModId));
    fDst.Write(Offsets[n].NamId, sizeof(Offsets[n].NamId));
  end;
  Offsets := nil;
  //
  fDst.Write(CurrOffset, sizeof(CurrOffset));
end;

end.
