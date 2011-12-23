procedure droite( x, y, xx, yy, coul : integer );
      var
         step,i : integer;
         a,b : real;
         xr,yr,xro,yro : real;
   begin
     xr:=x; yr:=y; xro:=xx; yro:=yy;
     (*writeln(le_file,'(',x:4,',',y:4,') ==> (',xx:4,',',yy:4,')');*)
     if abs(y-yy) > abs(x-xx) then
        begin
          a:= (x-xx) / (y-yy);
          b:= ( yr*xro - yro*xr) / (y-yy);
          i:=y;
          if y>yy then step:=-1
                  else step:=1;
          repeat
            putpix(Gd, trunc(a*i+b), i, coul);
            i:=i+step;
          until i=yy;
          (*writeln(le_file,'X == ',a:3:3,' * Y + ',b:3:3);*)
        end
      else
        begin
          a:= (y-yy) / (x-xx);
          b:= ((yro*xr) - (yr*xro)) / (x-xx);
          i:=x;
          if x>xx then step:=-1
                  else step:=1;
          repeat
            putpix(Gd,i, trunc(a*i+b), coul);
            i:=i+step;
          until i=xx;
          (*writeln(le_file,'Y == ',a:3:3,' * X + ',b:3:3);*)
        end;
   end;