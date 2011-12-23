  procedure rot_chariot;
     begin
       c1:= c2;
       c2:= c3;
       with c3 do
          begin
            val:= 32;
            code:= 9;
          end;
     end;

   procedure init_chariot;
     begin
       with c3 do
          begin
            rep:= 0;
            freq:= 0;
            acc:= 0;
          end;
       rot_chariot;
       rot_chariot;
     end;


procedure trait_ph;
     const  deca : array[0..2] of integer
                  =(300,30,40);
   begin
     ptr_tcph:= pred(num_ph);
     ledeb:=swap(t_cph[ptr_tcph]) + deca[typlec];
     lefin:=swap(t_cph[ptr_tcph+1]) + deca[typlec];
     nb_word:= lefin - ledeb;
     ptr_tcph:=ledeb shr 1;
     ptr_word := 0;
     repeat
       memw[adword: ptr_word]:= t_cph[ptr_tcph];
       ptr_word := ptr_word +2;
       ptr_tcph := ptr_tcph+1;
     until ptr_tcph>=lefin shr 1;

     ptr_oct:= 0;
     ptr_word:= 0;
     init_chariot;

     repeat
       rot_chariot;
       charg_car;
       trait_car;
     until ptr_word >= nb_word;

     rot_chariot;
     trait_car;
     entroct(ord('#'));
   end;



   procedure parole(rep, ht, typ : integer);
       var
          savph:array[0..500] of integer;
          i: integer;
          tempo:integer;

    begin
      if sonoff then exit;
      num_ph:= rep;
      haut:= ht;
      typlec:= typ;
      if typlec<>0 then
         begin
           for i:=0 to 500 do savph[i]:=t_cph[i];
           tempo:= tempo_bruit;
         end
                   else
          if haut>5 then tempo:= tempo_f
                    else tempo:= tempo_m;
      addfix:= (tempo-addv[0])/256;
      cctable(tbi);
      case typ of
              1 : begin
                    charge_bruit;
                    (*if zuul then zzuul(adbruit,0,1095);*)
                    regenbruit;
                  end;
               2 : begin
                    charge_son;
                    charge_phbruit;
                  end;
      end;
      trait_ph;
      litph(Tbi,typ,tempo);
      if typlec<>0 then
         for i:=0 to 500 do
            begin
              t_cph[i]:=savph[i];
              mlec:=typlec;
            end;
      writepal(numpal);
    end;



