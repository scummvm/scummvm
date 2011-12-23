function detect:integer;
  const s:string[10]='1986 Amstr';
  var i : integer;

begin
  Gd:=ega;
  hirs;
  mem[$a000:15920]:=37;
  if mem[$a000:15920]<>37 then
     begin
       clrscr;
       i:=1;
       while (i<10) and (s[i]=chr(mem[$fc01:i])) do
            i:=i+1;
       if i>8 then Gd:=ams
              else Gd:=cga;
     end;
  detect:=Gd
end;
