procedure dem2;
  var f:file;
      k:integer;
begin
  { -- demande de disk 2 -- }
  while ioresult<>0 do;
  assign(f,'mort.001');
  {$i-}
  reset(f);
  while ioresult<>0 do
   begin
     show_mouse;
     k:= do_alert(al_mess2,1);
     hide_mouse;
     reset(f);
   end;
  close(f);
end;

procedure ani50;
  begin
    crep:= animof(1,1);
    PictOut(adani,crep,63,12);
    crep:= animof(2,1);
    PictOut(adani,crep,63,12);
    f2_all:= (res=1);
    repon(2, c_paroles+143);
  end;

(*overlay procedure apzuul;
  begin
    if (c_zzz=1) or (c_zzz=2) then
     begin
       zzuul(adcfiec+161,0,1644);
       c_zzz:=succ(c_zzz);
     end;
  end;*)

overlay procedure aff50(c:boolean);
    var k:integer;
  begin
    caff:= 50;
    maff:=0;
    taffich;
    dessine(ades,63,12);
    if c then ani50
         else repon(2, c_paroles+142);
    f3f8;
  end;

overlay procedure init_menu;
    var i, j, tai : integer;
        st        : phrase;
        f : file;
  begin
    assign(f,'menufr.mor');
    {$i-}
    reset(f,144);
    blockread(f,lettres,1);
    close(f);
    { ----  Demande de changement de disquette  ---- }
    dem2;


    for i:= 1 to 8 do
       inv[i]:= '*                     ';
    inv[7]:= '< -*-*-*-*-*-*-*-*-*- ';
    for i:= 1 to 7 do
       dep[i]:= '*                       ';
    i:= 1;
    repeat
      DeLine(i+c_action,st,tai);
      act[i]:= delig + '          ';
      if i<9 then
         begin
           if i<6 then
              begin
                DeLine(i+c_saction,st,tai);
                self[i]:= delig + '       ';
              end;
           DeLine(i+c_dis,st,tai);
           dis[i]:= delig+ ' ';
         end;
      i:= succ(i);
    until i= 22;
    for i:= 1 to 8 do
      begin
       disc[i]:=$500+i;
       if i<8 then depl[i]:=$200+i;
       invt[i]:=$100+i;
       if i>6 then menu_disable(invt[i]);
      end;
    msg3:=no_choice;
    msg4:=no_choice;
    msg[3]:=no_choice;
    msg[4]:=no_choice;
    clic:=false;
  end;


overlay procedure charpal;
   var f : file of tabdb;
       ft : file of tfxx;
       i,j,k : integer;
       fb : file of byte;
       b : byte;
begin
  assign(ft,'fxx.mor');
  {$i-}
  reset(ft);
  if ioresult<>0 then
           begin
             caff:=do_alert(err_mess,1);
             halt;
           end;
  read(ft,l);
  close(ft);
  assign(f,'plxx.mor');
  reset(f);
  for i:= 0 to 90 do read(f,tabpal[i]);
  close(f);
  assign(fb,  'cxx.mor');
  reset(fb);
  for j:=0 to 90 do
    begin
      read(fb,palcga[j].p);
      for i:=0 to 15 do
        with palcga[j].a[i] do
          begin
            read(fb,b);
            n:=b shr 4;
            hom[0]:= b shr 2 and 3;
            hom[1]:= b and 3;
          end;
    end;
  palcga[10].a[9]:= palcga[10].a[5];
  for j:=0 to 14 do
    begin
      read(fb,tpt[j].tax);
      read(fb,tpt[j].tay);
      for i:=1 to 20 do
        for k:=1 to 20 do
          read(fb,tpt[j].des[i,k]);
    end;
  close(fb);
end;

overlay procedure chartex;
  var i:integer;
      FiByte:file;
      s : phrase;
begin
(* debug('o3 chartex'); *)
  i:=0;
  assign(FiByte,'TXX.INP');
  {$i-}
  reset(FiByte);
  blockread(FiByte,T_mot,125);
  close(FiByte);
  assign(sauv_T,'TXX.NTP');
  reset(sauv_T);
  repeat
    read(sauv_T,T_rec[i]);
    i:=i+1;
  until eof(sauv_T);
  close(sauv_T);
  DeLine(578,s,i);
  al_mess:= delig;
  DeLine(579,s,i);
  err_mess:= delig;
  DeLine(580,s,i);
  ind_mess:= delig;
  DeLine(581,s,i);
  al_mess2:= delig;
end;

overlay procedure dialpre;

var
 cy, tay  : integer;
 st : phrase;
 ix : real;
 ch : char;

begin
 (* debug('o3 dialpre'); *)
 cy:=0;
 clrscr;
 textcolor(9);
 repeat
   cy:= cy+1;
   DeLine( cy+c_dialpre,st,tay);
   gotoxy(40-tay div 2,wherey+1);
   write(delig);
 until cy=20;
 ix:=0;
 repeat
   ix:= ix+1;
 until (keypressed or (ix=5e5));
 crep:= 998;
 textcolor(1);
 gotoxy(1,21); clreol;
 gotoxy(1,23);
 write('CARTE GRAPHIQUE      CGA    EGA    HERCULE/AT&T400    TANDY    AMSTRAD1512');
 gotoxy(12,24);
 write('Ctrl       C      E            H             T           A');
 repeat
   read(kbd,ch);
 until ch in [#1,#3,#5,#20,#8];
 case ch of
      #1,#3,#5 : Gd:= ord(ch) shr 1;
            #8 : Gd:= Her;
           #20 : Gd:= Tan;
 end;
 gotoxy(1,24);
 clreol;
 gotoxy(1,23);
 clreol;
 gotoxy(26,23);
 write('Jeu au Clavier / … la Souris');
 textcolor(4);
 gotoxy(33,23); write('C');
 gotoxy(48,23); write('S');
 repeat
   read(kbd,ch);
 until upcase(ch) in ['C','S'];
 int_m:= (upcase(ch)='S');
end;

overlay procedure init_lieu;
 var
   f_lieu : file of tab_mlieu;
begin
(* debug('o3 init_lieu'); *)
  assign(f_lieu,'MXX.mor');
  {$i-}
  reset(f_lieu);
  read(f_lieu,v_lieu);
  close(f_lieu);
end;


overlay procedure music;
 var
       fic : file;
         k : integer;
       fin : boolean;
        ch : char;
        x,y : real;

begin
  (* debug('o3 music'); *)
  if sonoff then exit;
  rech_cfiec:= True;
  assign(fic,'mort.img');
  {$i-}
  reset(fic);
  blockread(fic,mem[$3800:0],500);
  blockread(fic,mem[$47A0:0],123);
  close(fic);
  demus($3800,$5000,623);
  addfix:= (tempo_mus-addv[1])/256;
  cctable(tbi);

  fin:=false;
  k:=0;
  repeat
    fin:= keypressed;
    musyc( tbi, 9958 , tempo_mus);
    k:= k+1;
    fin:= fin or keypressed or (k>=5);
  until fin;
  while keypressed do read(kbd,ch);
end;


overlay procedure charge_bruit5;
     var f:file;
   begin
     assign(f,'bruit5');
     {$i-}
     reset(f);
     if ioresult<>0 then
     begin
       caff:=do_alert(err_mess,1);
       halt;
     end;
     blockread(f,mem[adbruit5:0],149);
     (*blockread(f,mem[$5CB0:0],100);
     blockread(f,mem[$3D1F:0],49);*)
     close(f);
   end;

overlay procedure charge_cfiec;
        var
            f : file;
      begin
        assign(f,'cfiec.mor');
       {$i-}
        reset(f);
        blockread(f,mem[adcfiec:0],511);
        blockread(f,mem[adcfiec+4088:0],311);
        close(f);
        rech_cfiec:= False;
      end;


overlay procedure charge_cfiph;
        var
            f : file;
      begin
        assign(f,'cfiph.mor');
        {$i-}
        reset(f,256);
        blockread(f,t_cph,50);
        close(f);
      end;


overlay procedure suite;
  var cpr:string[25];
begin
  hirs;
  repon(7,2035);
  caff:= 51;
  taffich;
  teskbd;
  if newGd<>Gd then Gd:=newGd;
  hirs;
  dessine(ades,0,0);
  gotoxy(20*pred(res)+8,24);
  textcolor(7);
  cpr:='COPYRIGHT 1989 : LANKHOR';
  if gd in [Ega,Ams,Cga] then write(cpr)
             else begin
                    putxy(104+72*res,190);
                    writeg(cpr,0);
                  end;
end;
