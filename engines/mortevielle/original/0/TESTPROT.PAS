{$i var.mor}
function testprot : boolean;
var
 regs : registers;
 buffer : array[0..511] of char;
 i,j,k : integer;
 test : array[0..2] of boolean;
 chaine : array[0..7] of char;
begin
 test[0]:=false;test[1]:=false;test[2]:=false;k:=0;
 chaine[0]:=' ';
 chaine[1]:='M';
 chaine[2]:='A';
 chaine[3]:='S';
 chaine[4]:='T';
 chaine[5]:='E';
 chaine[6]:='R';
 chaine[7]:=' ';
 with regs do
  begin
   while (test[k]=false) and (k<2) do
    begin
     k:=k+1;
     ax:=0;intr(19,regs);
     ax:=$0201;cx:=$0001;dx:=$0100+k-1;es:=seg(buffer);bx:=ofs(buffer);
     intr(19,regs);
     test[k]:=true;
     i:=0;
     while (test[k]) and (i<19) do
      begin
       ax:=$0201;cx:=$2700+i;dx:=$0100+k-1;es:=seg(buffer);bx:=ofs(buffer);
       intr(19,regs);if lo(flags) mod 2=1 then test[k]:=false;i:=i+1;
      end;
      for j:=0 to 7 do
       if buffer[j+504]<>chaine[j] then test[k]:=false;
    end;
  end;
 testprot:=(test[1] or test[2]);
end;



BEGIN
WRIteLN(testprot);
END.