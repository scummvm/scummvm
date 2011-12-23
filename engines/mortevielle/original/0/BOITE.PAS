procedure boite(x,y,dx,dy,coul : integer);
   var i : integer;
      xi,yi,xo,yo : integer;
 begin
   (* debug('boite'); *)
   hide_mouse;
   if res=1 then
      begin
        x:= x shr 1;
        dx:= dx shr 1;
      end;
   xi:=x; yi:=y; xo:=x; yo:=y;
   for i:=0 to dx+dy do
      begin
        putpix(Gd,xi,yi,coul);
        if xi=x+dx then begin
                          if Gd<>cga then putpix(Gd,pred(xi),yi,coul);
                          yi:=succ(yi);
                        end
                   else xi:=succ(xi);
        putpix(Gd,xo,yo,coul);
        if yo=y+dy then xo:=succ(xo)
                   else begin
                          if Gd<>cga then putpix(Gd,succ(xo),yo,coul);
                          yo:=succ(yo);
                        end;
      end;
   show_mouse;
 end;
