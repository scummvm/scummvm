overlay function do_alert(str:str255; n:integer):integer;

   const nligne=7;
         coord: array[1..2,1..3] of integer
               =( (150, 72, 103),
                  (143, 107, 183) );
   var coldep,esp,i,l,nbcase,quoi,ix:integer;
       st, chaine:str255;
       Limit: array[1..2,1..2]  of integer;
       c,dumi:char;
       s:array[1..2] of string[3];
       cx,cy,cd,nbcol,nblig:integer;
       touch,newaff,test,test1,test2,test3,dum : boolean;
       cas : str30;

     procedure decod(s:str255; var nbc,nbl,col : integer; var c : str255; var cs:str30);
           var i,k : integer;
               v : boolean;
         begin
           val(s[2],nbc,i);
           c:= '';
           nbl:= 0; i:= 5; k:= 0;
           v:=True; col:=0;

           while s[i]<>']' do
             begin
               c:=c+s[i];
               if (s[i]='|') or (s[i+1]=']') then
                  begin
                    if k>col then col:=k;
                    k:= 0;
                    nbl:=nbl+1;
                  end
                else if s[i]<>' ' then v:=False;
               i:=i+1;
               k:=k+1;
             end;
           if v then begin
                       c:='';
                       col:=20;
                     end
                else begin
                       c:=c+']';
                       col:=col+6;
                     end;
           i:=i+1;
           cs:=copy(s,i,30);
           if res=2 then col:= col*6
                    else col:= col*10;
         end;

     procedure posit(ji:integer);
        begin
          putxy(coldep + (40+esp) *pred(ji),98);
        end;

     procedure fait_boite(lidep,nli,tx:integer);
          var x,y,xx,yy:integer;
        begin
          if tx>640 then tx:=640;
          x:= 320- tx shr 1;
          y:=pred(lidep) shl 3;
          xx:=x+ tx;
          yy:=y + nli shl 3;
          box(15,Gd,x,y,xx,yy,255);
          box(0,Gd,x,y+2,xx,y+4,255);
          box(0,Gd,x,yy-4,xx,yy-2,255);
        end;

     procedure fait_choix(c:str30);
          var i,l,x:integer;
              ch:char;
        begin
          i:=1;
          x:=coldep;
          for l:=1 to nbcase do
            begin
              s[l]:='';
              repeat
                i:=i+1;
                ch:=c[i];
                s[l]:=s[l]+ch;
              until c[i+1]=']';
              i:=i+2;
              while length(s[l])<3 do s[l]:=s[l]+' ';
              putxy(x,98);
              writeg(' '+s[l]+' ',0);
              x:= x+esp+40;
            end;
        end;

begin
  (*debug('** do_alert **');*)
  hide_mouse;
  while keypressed do read(kbd,dumi);
  clic:=false;
  decod(str,nbcase,nblig,nbcol,chaine,cas);
  sauvecr(50,succ(nligne) shl 4);

  i:=0;
  if chaine='' then
    begin
      fait_boite(10,5,nbcol);
    end
   else
    begin
      fait_boite(8,7,nbcol);
      i:=0;
      ywhere:=70;
      repeat
        cx:= 320;
        st:= '';
        while not (chaine[i+1] in [#124,#93]) do
          begin
            i:=i+1;
            st:=st+chaine[i];
            if res=2 then cx:=cx-3
                     else cx:=cx-5;
          end;
        putxy(cx,ywhere);
        ywhere:=ywhere+6;
        writeg(st,4);
        i:=i+1;
      until chaine[i]=']';
    end;
  if nbcase=1 then esp:= nbcol- 40
              else esp:= (nbcol-nbcase*40) shr 1;
  coldep:=320-nbcol shr 1+esp shr 1;
  fait_choix(cas);
  limit[1,1]:=(coldep) shr 1*res;
  limit[1,2]:=limit[1,1]+40;
  if nbcase=1 then
     begin
       limit[2,1]:=limit[2,2];
     end
   else
     begin
       limit[2,1]:=(320+esp shr 1) shr 1*res;
       limit[2,2]:=(limit[2,1])+40;
     end;
  show_mouse;
  quoi:=0;
  dum:=false;
  repeat
    dumi:=#255;
    mov_mouse(dum,dumi);
    cx:= X_S;
    cy:= Y_S;
    test:=(cy>95) and (cy<105);
    newaff:=false;
    if test then
     begin
       test1:=(cx>limit[1,1]) and (cx<limit[1,2]);
       test2:=test1;
       if nbcase>1 then test2:= test1 or ((cx>limit[2,1]) and (cx<limit[2,2]));
       if test2 then
        begin
          newaff:=true;
          if test1 then ix:=1
                   else ix:=2;
          if (ix<>quoi)  then
           begin
             hide_mouse;
             if quoi<>0 then
              begin
                posit(quoi);
                writeg(' '+s[quoi]+' ',0);
              end;
             posit(ix);
             writeg(' '+s[ix]+' ',1);
             quoi:=ix;
             show_mouse;
           end;
        end;
     end;
    if (quoi<>0) and not newaff then
     begin
       hide_mouse;
       posit(quoi);
       writeg(' '+s[quoi]+' ',0);
       quoi:=0;
       show_mouse;
     end;
     test3:=(cy>95) and (cy<105) and (((cx>limit[1,1]) and (cx<limit[1,2]))
                                   or ((cx>limit[2,1]) and (cx<limit[2,2])));
  until clic;
  clic:=false;
  hide_mouse;
  if not test3 then begin
                  quoi:=n;
                  posit(n);
                  writeg(' '+s[n]+' ',1);
                end;
  charecr(50,succ(nligne) shl 4);
  show_mouse;
  do_alert:=quoi;
end;
