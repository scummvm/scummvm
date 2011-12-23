procedure fill_box(x,y,dx:integer);
begin
  box(15,Gd,pred(x) shl 3, pred(y) shl 3, pred(x+dx) shl 3, y shl 3, 255);
end;

procedure clear_box(x,y,dx:integer);
begin
  box(0,Gd,pred(x) shl 3, pred(y) shl 3, pred(x+dx) shl 3, y shl 3, 255);
end;

procedure writeg(l:str255;c:integer);

 var i, x, xo, yo : integer;
     cecr : integer;
     t:boolean;
begin
(*  debug('writeg : '+l);*)

  if l='' then exit;
  hide_mouse;
  xo:=xwhere;
  yo:=ywhere;
  if res=2 then i:=6
           else i:=10;
  x:=xo+i*length(l);
  case c of
          1,3  : begin
                   cecr:=0;
                   box(15,Gd,xo,yo,x,yo+7,255)
                 end;
             4 : begin
                   cecr:=0;
                 end;
             5 : begin
                   cecr:=15;
                 end;
           0,2 : begin
                   cecr:=15;
                   box(0,Gd,xo,yo,x,yo+7,255);
                 end;
  end;
  xo:=xo+1;
  yo:=yo+1;
  for x:=1 to length(l) do
     begin
       affcar(Gd,xo,yo,cecr,ord(l[x]));
       xo:=xo+i;
     end;
  show_mouse;
end;
