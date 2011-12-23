
   procedure spfrac(wor : integer);
     begin
       c3.rep:= wor shr 12;
       if (typlec=0) and (c3.code<>9) then
          if ((c3.code>4) and (c3.val<>20) and not (c3.rep in [3,6,9])) or
             ((c3.code<5) and not (c3.val in [19,22]) and not (c3.rep in [4,9])) then
             c3.rep:= c3.rep + 1;
       c3.freq:= (wor shr 6) and 7;
       c3.acc:= (wor shr 9) and 7;
     end;

   procedure charg_car;
       var wor, int : integer;
     begin
       wor:= swap(memw[adword:ptr_word]);
       int:= wor and $3F;
       case int of
                  60 : begin
                         c3.val:= 32;  (*  " "  *)
                         c3.code:= 9;
                       end;
                  61 : begin
                         c3.val:= 46;  (*  "."  *)
                         c3.code:= 9;
                       end;
                  62 : begin
                         c3.val:= 35;  (*  "#"  *)
                         c3.code:= 9;
                       end;
              22..47 : begin
                         int:= int-22;
                         c3.val:= int;
                         c3.code:= typcon[int];
                       end;
              48..56 : begin
                         c3.val:= int-22;
                         c3.code:= 4;
                       end;
              14..21 : begin
                         c3.val:= int;
                         c3.code:= 6;
                       end;
               0..13 : begin
                         c3.val:= int;
                         c3.code:= 5;
                       end;
       end;
       spfrac(wor);
       ptr_word:= ptr_word+2;
     end;


   procedure entroct( o:byte);
     begin
       mem[adtroct:ptr_oct]:= o;
       ptr_oct:= ptr_oct+1;
     end;

   procedure veracf(b:byte);
     begin
     end;

overlay procedure cctable (var t:tablint);
  var
      k:integer;
      tb : array[0..256] of real;

   begin
     Tb[0]:=0;
     for k:=0 to 255 do
     begin
       Tb[k+1]:= addfix + Tb[k];
       T[255-k]:=trunc(Tb[k])+1;
     end;
   end;

overlay procedure regenbruit;
        var
            i,j : integer;
      begin
        i:= offsetb3+8590;
        j:= 0;
        repeat
          t_cph[j]:=memw[adbruit3:i];
          i:= i+2;
          j:= j+1;
        until i>= offsetb3+8790
      end;

overlay   procedure charge_son;
        var
            f : file;
      begin
        assign(f,'sonmus.mor');
        reset(f);
        blockread(f,mem[$7414:0],273);
        (*blockread(f,mem[adson:0],300);
        blockread(f,mem[adson+2400:0],245);*)
        demus($7414,adson,273);
        close(f);
      end;

overlay   procedure charge_phbruit;
        var
            f : file;
      begin
        assign(f,'phbrui.mor');
        reset(f);
        blockread(f,t_cph,3);
        close(f);
      end;

overlay   procedure charge_bruit;
        var
            f : file;
            j,i:integer;
      begin
        assign(f,'bruits');
        reset(f);
        blockread(f,mem[adbruit:0],250);
        for i:=0 to 19013 do mem[adbruit:32000+i]:= mem[adbruit5:i];
        blockread(f,mem[adbruit1:offsetb1],149);
        close(f);
      end;

overlay   procedure trait_car;
       var    d3 : byte;
           d2, i : integer;
     begin
       case c2.code of
                    9 : if (c2.val<>ord('#')) then for i:=0 to c2.rep do entroct(c2.val);

                  5,6 : begin
                          if c2.code=6 then d3:= tabdph[(c2.val - 14) shl 1]
                                       else d3:= null;
                          if (c1.code>=5) then
                             begin
                               veracf(c2.acc);
                               if (c1.code=9) then
                                  begin
                                    entroct(4);
                                    if d3=null then entroct(c2.val)
                                               else entroct(d3);
                                    entroct(22);
                                  end;
                             end;
                          case (c2.rep) of
                                        0 : begin
                                              entroct(0);
                                              entroct(c2.val);
                                              if d3=null then if c3.code=9 then entroct(2)
                                                                           else entroct(4)
                                                         else if c3.code=9 then entroct(0)
                                                                           else entroct(1);
                                            end;
                                    4,5,6 : begin
                                              if c2.rep<>4 then
                                                 begin
                                                   i:= c2.rep-5;
                                                   repeat
                                                     i:= i-1;
                                                     entroct(0);
                                                     if d3=null then entroct(c2.val)
                                                                else entroct(d3);
                                                     entroct(3);
                                                   until i<0;
                                                 end;
                                              if d3=null then
                                                 begin
                                                   entroct(4);
                                                   entroct(c2.val);
                                                   entroct(0);
                                                 end
                                                else
                                                 begin
                                                   entroct(0);
                                                   entroct(c2.val);
                                                   entroct(3);
                                                 end;
                                            end;
                                    7,8,9 : begin
                                              if c2.rep<>7 then
                                                 begin
                                                   i:= c2.rep-8;
                                                   repeat
                                                     i:= i-1;
                                                     entroct(0);
                                                     if d3=null then entroct(c2.val)
                                                                else entroct(d3);
                                                     entroct(3);
                                                   until i<0;
                                                 end;
                                              if d3=null then
                                                 begin
                                                   entroct(0);
                                                   entroct(c2.val);
                                                   entroct(2);
                                                 end
                                                else
                                                 begin
                                                   entroct(0);
                                                   entroct(c2.val);
                                                   entroct(0);
                                                 end;
                                            end;
                                    1,2,3 : begin
                                              if c2.rep<>1 then
                                                 begin
                                                   i:= c2.rep-2;
                                                   repeat
                                                     i:= i-1;
                                                     entroct(0);
                                                     if d3=null then entroct(c2.val)
                                                                else entroct(d3);
                                                     entroct(3);
                                                   until i<0;
                                                 end;
                                              entroct(0);
                                              entroct(c2.val);
                                              if c3.code=9 then entroct(0)
                                                           else entroct(1);
                                            end;
                          end;  (*  case  c2.rep  *)
                        end;

                  2,3 : begin
                          d3:= c2.code+5;  (*  7 ou 8  => voyelle correspondante  *)
                          if c1.code>4 then
                             begin
                               veracf(c2.acc);
                               if c1.code=9 then
                                  begin
                                    entroct(4);
                                    entroct(d3);
                                    entroct(22);
                                  end;
                             end;
                          i:=c2.rep;
                          if i<>0 then
                             begin
                               repeat
                                 i:=i-1;
                                 entroct(0);
                                 entroct(d3);
                                 entroct(3);
                               until i<=0;
                             end;
                          veracf(c3.acc);
                          if c3.code=6 then
                             begin
                               entroct(4);
                               entroct(tabdph[(c3.val-14) shl 1]);
                               entroct(c2.val);
                             end
                            else
                             begin
                               entroct(4);
                               if c3.val=4 then entroct(3)
                                           else entroct(c3.val);
                               entroct(c2.val);
                             end;
                        end;
                  0,1 : begin
                          veracf(c2.acc);
                          case c3.code of
                                       2 : d2:= 7;
                                       3 : d2:= 8;
                                       6 : d2:= tabdph[(c3.val-14) shl 1];
                                       5 : d2:= c3.val;
                                      else d2:=10;
                          end;    (*  case  c3.code  *)
                          d2:= d2 *26 + c2.val;
                          if tnocon[d2]=0 then d3:=2
                                          else d3:=6;
                          if c2.rep>=5 then
                             begin
                               c2.rep:=c2.rep-5;
                               d3:= 8-d3;         (*  echange 2 et 6  *)
                             end;
                          if c2.code=0 then
                             begin
                               i:=c2.rep;
                               if i<>0 then
                                  begin
                                    repeat
                                      i:=i-1;
                                      entroct(d3);
                                      entroct(c2.val);
                                      entroct(3);
                                    until i<=0;
                                  end;
                               entroct(d3);
                               entroct(c2.val);
                               entroct(4);
                             end
                            else
                             begin
                               entroct(d3);
                               entroct(c2.val);
                               entroct(3);
                               i:=c2.rep;
                               if i<>0 then
                                  begin
                                    repeat
                                      i:=i-1;
                                      entroct(d3);
                                      entroct(c2.val);
                                      entroct(4);
                                    until i<=0;
                                  end;
                             end;
                          if c3.code=9 then
                             begin
                               entroct(d3);
                               entroct(c2.val);
                               entroct(5);
                             end
                            else
                             if not (c3.code in [0..1,4]) then
                                begin
                                  veracf(c3.acc);
                                  case (c3.code) of
                                                 3 : d2:= 8;
                                                 6 : d2:= tabdph[(c3.val-14) shl 1];
                                                 5 : d2:= c3.val;
                                                else d2:= 7;
                                  end;  (*  case c3.code  *)
                                  if d2=4 then d2:=3;
                                  if intcon[c2.val]<>0 then c2.val:= c2.val+1;
                                  if (c2.val=17) or (c2.val=18) then c2.val:=16;
                                  entroct(4);
                                  entroct(d2);
                                  entroct(c2.val);
                                end;
                        end;
                    4 : begin
                          veracf(c2.acc);
                          i:=c2.rep;
                          if i<>0 then
                             begin
                               repeat
                                 i:=i-1;
                                 entroct(2);
                                 entroct(c2.val);
                                 entroct(3);
                               until i<=0;
                             end;
                          entroct(2);
                          entroct(c2.val);
                          entroct(4);
                          if c3.code=9 then
                             begin
                               entroct(2);
                               entroct(c2.val);
                               entroct(5);
                             end
                            else
                             if not (c3.code in [0..1,4]) then
                                begin
                                  veracf(c3.acc);
                                  case (c3.code) of
                                                 3 : d2:= 8;
                                                 6 : d2:= tabdph[(c3.val-14) shl 1];
                                                 5 : d2:= c3.val;
                                                else d2:= 7;
                                  end;  (*  case c3.code  *)
                                  if d2=4 then d2:=3;
                                  if intcon[c2.val]<>0 then c2.val:= c2.val+1;
                                  entroct(4);
                                  entroct(d2);
                                  entroct(tabdbc[(c2.val-26) shl 1+1]);
                                end;
                        end;
       end;  (* case c2.code  *)
     end;

{overlay function testprot : boolean;
var
   regs : registres;
 buffer : array[0..511] of char;
  i,j,k : integer;
     st : phrase;
    tay : integer;
   test : array[0..2] of boolean;
const
 chaine : array[0..7] of char
        = (' ','M','A','S','T','E','R',' ');

begin
 test[0]:= false;test[1]:=false;test[2]:=false;k:=0;
 with regs do
 begin
   while (test[k]=false) and (k<2) do
   begin
     k:=k+1;
     ax:=0;intr(19,regs);
     ax:=$0201;cx:=$0001;dx:=$0100+k-1;es:=seg(buffer);bx:=ofs(buffer);
     intr(19,regs);
     test[k]:=true;
     tesok:= true;
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
}

