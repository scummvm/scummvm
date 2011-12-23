(* NIVEAU 14*)

overlay procedure menut(no : integer; nom: str30);
     var h,l:byte;
         s:str30;

   begin
     (* debug('menut'); *)
     h:=hi(no);
     l:=lo(no);
     s:=nom;
     if not tesok then
        begin
          clrscr;
          halt;
        end;
     while length(s)<30 do s:=s+' ';
     case h of
           invent  :  if l<>7 then inv[l]:= ' '+s;
           depla   :  dep[l]:= s;
           action  :  act[l]:= s;
           saction :  self[l]:= s;
           discut  :  dis[l]:= s;
        end;
  end;

overlay procedure menu_disable(no:integer);
     var h,l:byte;
   begin
     (* debug('menu_disable'); *)
     h:=hi(no);
     l:=lo(no);
     case h of
           invent : begin
                 if l>6 then begin
                               inv[l][1]:='<';
                               inv[l][22]:='>';
                             end
                        else inv[l][1]:='*';
               end;
           depla : dep[l][1]:='*';
           action : act[l][1]:='*';
           saction : self[l][1]:='*';
           discut : dis[l][1]:='*';
        end;
   end;

overlay procedure menu_enable(no:integer);
     var h,l:byte;
   begin
     (* debug('menu_disable'); *)
     h:=hi(no);
     l:=lo(no);
     case h of
           invent : begin
                      inv[l][1]:=' ';
                      inv[l][22]:=' ';
                    end;
           depla : dep[l][1]:=' ';
           action : act[l][1]:=' ';
           saction : begin
                       self[l][1]:=' ';
                       self[l][1]:=' ';
                     end;
           discut : dis[l][1]:=' ';
        end;
   end;

procedure menu_aff;
     var
         ind_tabl,k,col:integer;
         c: char;
         pt,x,y,color,msk,
         num_letr: integer;
   begin
     (* debug('menu_aff'); *)
     hide_mouse;
     {if not tesok then
        begin
          clrscr;
          halt;
        end;}
     box( 7, Gd, 0,0, 639,10, 255);
     col:= 28*res;
     if Gd = cga then color:=1
                 else color:=9;
     num_letr:=0;
     repeat     (* lettre par lettre *)
       num_letr:= num_letr +1;
       ind_tabl:=0;
       y:= 1;
       repeat    (* colonne par colonne *)
         k:=0;
         x:= col;
         repeat   (* ligne par ligne *)
           msk:=$80;
           for pt:=0 to 7 do
            begin
              if (lettres[num_letr,ind_tabl] and msk)<>0 then
                 begin
                   putpix(Gd,x+1,y+1,0);
                   putpix(Gd,x,y+1,0);
                   putpix(Gd,x,y,color);
                 end;
              msk:= msk shr 1;
              x:=x +1;
            end;
           ind_tabl:=succ(ind_tabl);
           k:=succ(k);
         until k=3;
         y:=y+1;
       until (y=9);
       col:=col+48*res;
     until num_letr=6;
     show_mouse;
   end;


procedure draw_menu;
  begin
    (* debug('draw_menu'); *)
    menu_aff;
    active_menu:=true;
    msg4:=no_choice;
    msg3:=no_choice;
    choisi:=false;
    clic:=false;
    test0:=false;
  end;

procedure invers(ix:integer);
     var s:string[23];
   begin
     (* debug('invers'); *)
     if msg4=no_choice then exit;
     putxy(don[msg3,1] shl 3,succ(lo(msg4)) shl 3);
     case msg3 of
                  1 : s:= inv[lo(msg4)];
                  2 : s:= dep[lo(msg4)];
                  3 : s:= act[lo(msg4)];
                  4 : s:= self[lo(msg4)];
                  5 : s:= dis[lo(msg4)];
                  6 : s:= fic[lo(msg4)];
                  7 : s:= fic[1]+' '+chr(48+lo(msg4));
                  8 : if lo(msg4)=1 then s:=recom
                              else s:= fic[2]+' '+chr(47+lo(msg4));
          end;
     if (s[1]<>'*') and (s[1]<>'<') then writeg(s,ix)
                                    else msg4:=no_choice;
   end;

procedure util(x,y:integer);
     var ymx, dxcar, xmn, xmx, ix:integer;
   begin
     (* debug('util'); *)
     ymx:= don[msg3,4] shl 3+16;
     dxcar:= don[msg3,3];
     xmn:= don[msg3,1] shl 2*res;
     if res=1 then ix:=5 else ix:=3;
     xmx:= dxcar*ix*res+ xmn+ 2;
     if (x>xmn) and (x<xmx) and (y<ymx) and (y>15) then
             begin
               ix:= pred(y shr 3)+ msg3 shl 8;
               if ix<>msg4 then
                 begin
                   invers(1);
                   msg4:=ix;
                   invers(0);
                 end;
             end
           else if msg4<>no_choice then
                 begin
                   invers(1);
                   msg4:=no_choice;
                 end;
   end;

procedure menu_down( ii :integer);
     var cx, xcc:integer;
         xco, nb_lig :integer;
   begin
     (* debug('menu_down'); *)
     xco:=don[ii,1];
     nb_lig:= don[ii,4];
     hide_mouse;
     sauvecr(10,succ(don[ii,2]) shl 1);
     xco:= xco shl 3;
     if res=1 then cx:=10 else cx:=6;
     xcc:= xco+ (don[ii,3]*cx)+6;
     box(15, Gd, xco,12, xcc, 10 + don[ii,2] shl 1, 255);
     box(0, Gd, xcc, 12, xcc+4, 10 + don[ii,2] shl 1,255);
     box(0,Gd,xco, 8 + don[ii,2] shl 1, xcc+4, 12 + don[ii,2] shl 1,255);
     putxy(xco,16);
     cx:=0;
     repeat
       cx:= succ(cx);
       case ii of
             1 : if inv[cx][1]<> '*' then writeg(inv[cx],4);
             2 : if dep[cx][1]<> '*' then writeg(dep[cx],4);
             3 : if act[cx][1]<> '*' then writeg(act[cx],4);
             4 : if self[cx][1]<>'*' then writeg(self[cx],4);
             5 : if dis[cx][1]<> '*' then writeg(dis[cx],4);
             6 : writeg(fic[cx],4);
             7 : writeg(fic[1]+' '+chr(48+cx),4);
             8 : if cx=1 then writeg( recom,4)
                         else writeg(fic[2]+' '+chr(47+cx),4);
          end;
       putxy(xco,ywhere+8)
     until cx=nb_lig;
     test0:=true;
     show_mouse;
   end;

procedure menu_up(xx:integer);
   begin
     (* debug('menu_up'); *)
     if test0 then
       begin
         charecr(10,succ(don[xx,2]) shl 1);
         test0:=false;
       end;
   end;

procedure erase_menu;
  begin
    (* debug('erase_menu'); *)
    active_menu:=false;
    clic:=false;
    menu_up(msg3);
  end;

procedure mdn;
       var x,y,c,a,ix:integer;
           tes:boolean;
   begin
     (* debug('mdn'); *)
     if not active_menu then exit;
     x:=X_S; y:=Y_S;
     if (not clic) then
      begin
        if (x=xprec) and
           (y=yprec) then exit
        else begin
               xprec:=x;
               yprec:=y;
             end;
        tes:=(y<11) and ((x in [28*res..28*res+24,76*res..76*res+24])
                         or ((x>124*res) and (x<124*res+24))
                         or ((x>172*res) and (x<172*res+24))
                         or ((x>220*res) and (x<220*res+24))
                         or ((x>268*res) and (x<268*res+24)));
        if tes then
           begin
             if x<76*res then ix:=invent
              else if x<124*res then ix:=depla
               else if x<172*res then ix:=action
                else if x<220*res then ix:=saction
                 else if x<268*res then ix:=discut
                  else ix:= fichier;
             if (ix<>msg3) or (not test0) then
              if not ((ix=fichier) and  (msg3 in [sauve, charge])) then
                 begin
                   menu_up(msg3);
                   menu_down(ix);
                   msg3:=ix;
                   msg4:=no_choice;
                 end;
          end
         else    { Not in the MenuTitle line }
          begin
            if (y>11) and (test0) then util(x,y);
          end
      end
     else          (* il y a eu 'clic' *)
     if (msg3=fichier) and (msg4<>no_choice) then
       begin
         clic:=false;
         menu_up(msg3);
         if lo(msg4)=1 then msg3:=7
                   else msg3:=8;
         menu_down(msg3);
       end
     else    (*  il y a eu clic sur un autre menu  *)
       begin
         choisi:=(test0) and (msg4<>no_choice);
         menu_up(msg3);
         msg[4]:=msg4;
         msg[3]:=msg3;
         msg3:=no_choice;
         msg4:=no_choice;
         clic:=false;
       end;
   end;

