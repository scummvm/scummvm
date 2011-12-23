var
    indis, valfix,evol : integer;
    un_huit:real;

overlay procedure ctable(indis,valfix:integer);

  var x,y : real;
      Tb:array[0..255] of real;
      k:integer;

begin
{-----------------------------------------------------------------------------}
{                           CALCUL DE LA TABLE                                }
{-----------------------------------------------------------------------------}
(*  coef:=un_huit;*)
  x:= freq0/indis;
  Tb[0]:= indis;

  for k:= 1 to 255 do
     begin
(*       if valfix=0 then
          begin
            x:=x*coef;
            coef:= coef * (1 + ln(evol));
            if evol<>1 then
               begin
               end;
          end
         else*)
       x:= x+ valfix;
       Tb[k]:=freq0/x;
       y:=freq0/x - trunc(freq0/x);
       if (y>0.5) then Tb[k]:=Tb[k]+1;

     end;

  for k:=0 to 255 do
     begin
       Tbi[k]:=trunc(Tb[k]);
     end;

end;
