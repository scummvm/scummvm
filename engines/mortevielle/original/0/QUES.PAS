
overlay function ques:boolean;
   const ta : array[1..10] of integer
             = ( 511, 516, 524, 531, 545,
                 552, 559, 563, 570, 576);
         OK : array[1..10] of integer
            = ( 4, 7, 1, 6, 4, 4, 2, 5, 3, 1 );

   var q, func, test : boolean;
       i,j,k,y,memk : integer;
       tay , tmax : integer;
       rep, prem, der : integer;
       st : phrase;
       key : char;
       Coor : mult_rect;
       Chaines : array[1..14] of str40;
       compte : integer;

begin
  test:=False;
  i:=0;
  compte:= 0;

  repeat
    hide_mouse;
    hirs;
    show_mouse;
    i:= i+1;
    DeLine(ta[i],st,tay);
    if res=1 then y:=29 else y:=23;
    box(15,Gd,0,14,630,y,255);
    Afftex(st,20,15,100,2,0);
    if i<>10 then
       begin
         prem:= ta[i]+1;
         der:= ta[i+1]-1;
       end
     else
       begin
         prem:= 503;
         der:= 510;
       end;
    y:=35;
    tmax:= 0;
    memk:= 1;
    for j:= prem to der do
      begin
        DeLine(j,st,tay);
        if (tay>tmax) then tmax:= tay;
        Afftex(st,100,y,100,1,0);
        Chaines[memk]:= delig;
        memk:= memk+1;
        y:= y+8;
      end;
    for j:= 1 to succ(der-prem) do
      begin
      with Coor[j] do
        begin
          x1:= 45*res;
          x2:= (tmax*3 + 55)*res;
          y1:= 27 + j*8;
          y2:= 34 + j*8;
          Etat:= True;
        end;
        while length(Chaines[j] )<tmax do
          begin
            Chaines[j]:= Chaines[j] +' ';
          end;
      end;
    Coor[j+1].Etat:= False;
    if res=1 then rep:=10 else rep:=6;
    boite(80, 33, 40+tmax*rep, (der-prem)*8+ 16, 15);
    rep:= 0;
    j:= 0;
    memk:= 0;
    repeat
      clic:= false;
      tesok:=false;
      mov_mouse(func,key);
      k:= 1;
      while Coor[k].Etat and not Dans_rect(Coor[k]) do k:= k+1;
      if Coor[k].Etat then
         begin
           if (memk<>0) and (memk<>k) then
              begin
                (*DeLine(T_rec[ta[i]+memk].indis,T_rec[ta[i]+memk].point,st,tay);*)
                for j:= 1 to tmax do st[j]:= Chaines[memk][j];
                st[1+tmax]:= '$';
                Afftex(st,100,27+memk*8,100,1,0);
              end;
           if memk<>k then
              begin
                (*DeLine(T_rec[pred(prem)+k].indis,T_rec[pred(prem)+k].point,st,tay);*)
                for j:= 1 to tmax do st[j]:= Chaines[k][j];
                st[1+tmax]:= '$';
                Afftex(st,100,27+k*8,100,1,1);
                memk:= k;
              end;
         end
       else
         if memk<>0 then
            begin
              for j:= 1 to tmax do st[j]:= Chaines[memk][j];
              st[1+tmax]:= '$';
              Afftex(st,100,27+memk*8,100,1,0);
              memk:=0;
            end;
    until (memk<>0) and clic;
    if memk=OK[i] then compte:=compte+1
                  else begin
                         if i=5 then i:=i +1;
                         if (i=7) or (i=8) then i:= 10;
                       end;
    if i=10 then q:= {testprot} true;
  until i=10;
  ques:= (compte=10) and q;
end;
