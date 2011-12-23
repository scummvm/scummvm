const
       m_arrow       = 0;
       m_point_hand  = 1;

var  int_m,
     clic : boolean;

     m_show,
     X_S,
     Y_S,
     P_O_S,
     mouse_shwn : integer;

     S_S  : array[0..5,0..11] of integer;


procedure init_mouse;
  var i, j : integer;
       reg : registres;
begin
  mouse_shwn:= 0;
  X_S:= 0;
  Y_S:= 0;
  P_O_S:= 0;
  (*int_m:= False;*)
  clic:= False;
  m_show:= m_arrow;
  if (memw[0:$cc]=0) and (memw[0:$ce]=0) then int_m:= False;
  if int_m then
   begin
    reg.ax:= 0;
    intr($33,reg);
    int_m:= (reg.ax=-1);
    if int_m then
      begin
       with reg do
         begin
           ax:=4;
           cx:=0;
           dx:=0;
         end;
       intr($33,reg);
     end;
   end;
end;

(*procedure change_mouse( t : integer );
begin
  m_show:= t;
end;*)

(*procedure dessine_souris(xf,yf:integer);
  var x, xx, y :integer;
begin
  case m_show of
             m_arrow : begin
                         affput(2,Gd,xf,yf,0,0);
                       end;
  end;
end;  *)

procedure hide_mouse;
  var i,j,k,ps,ds:integer;
      imp:boolean;
begin
  mouse_shwn := pred(mouse_shwn);
  if mouse_shwn=0 then
     begin
       imp:= odd(Y_S);
       j:=P_O_S;
       case Gd of
             cga : begin
                     k:=0;
                     j:=Y_S shr 1 * 80+ X_S shr 2;
                     repeat
                       memw[$b800:j]:=S_S[0,k];
                       memw[$b800:j+2]:=S_S[1,k];
                       memw[$ba00:j]:=S_S[2,k];
                       memw[$ba00:j+2]:=S_S[3,k];
                       j:=j+80;
                       k:= succ(k);
                     until k>=5;
                   end;
                ams : begin
                        for i:=0 to 3 do
                        begin
                          port[$3DD]:= 1 shl i;
                          k:=0;
                          j:=P_O_S;
                          repeat
                            if imp then
                              begin
                                memw[$b800:j]:=S_S[i,k];
                                j:=j+80-$2000;
                              end
                             else
                              begin
                                memw[$b800:j]:=S_S[i,k];
                                j:=j+$2000;
                              end;
                            imp:=not imp;
                            k:= succ(k);
                          until k>=8;
                        end;
                      end;
                ega : begin
                        port[$3C4]:=2;
                        port[$3CE]:=8;
                        port[$3CF]:=255;
                        i:= 0;
                        repeat
                          port[$3C5]:= 1 shl i;
                          k:=0;
                          j:=P_O_S;
                          repeat
                            ps:=mem[$A000:j];
                            mem[$A000:j]:=lo(S_S[i,k]);
                            ps:=mem[$A000:j+1];
                            mem[$A000:j+1]:=hi(S_S[i,k]);
                            j:=j+80;
                            k:= succ(k);
                          until k>=8;
                          i:= i+1;
                        until i=4;
                      end;
                her : begin
                        j:= Y_S shr 1 *80+X_S shr 3;
                        for i:=0 to 5 do
                         begin
                           for k:=0 to 3 do memw[$b000+k*$200:j]:=S_S[i,k];
                           j:=j+80;
                         end;
                      end;
                tan : begin
                        j:= Y_S shr 2 *160+X_S shr 1;
                        k:=0;
                        repeat
                          for i:=0 to 3 do
                           begin
                             memw[$b800+$200*i:j]:=S_S[k,i+k shl 2];
                             memw[$b800+$200*i:j+2]:=S_S[k+3,i+k shl 2];
                           end;
                          j:=j+160;
                          k:= succ(k);
                        until k=3;
                      end;

       end;  (* case Gd *)
     end;
end;

procedure show_mouse;
  var i, j, k, l,
      ps,ds : integer;
      imp:boolean;
      c : char;
begin
  mouse_shwn := mouse_shwn +1;
  if mouse_shwn<>1 then exit;
  j:=P_O_S;
  imp:= odd(Y_S);
  i:=X_S and 7;
  case Gd of
             cga : begin
                     k:=0;
                     j:=Y_S shr 1 * 80+ X_S shr 2;
                     repeat
                       S_S[0,k]:=memw[$b800:j];
                       S_S[1,k]:=memw[$b800:j+2];
                       S_S[2,k]:=memw[$ba00:j];
                       S_S[3,k]:=memw[$ba00:j+2];
                       j:=j+80;
                       k:= succ(k);
                     until k>=5;
                   end;
          ams : begin
                  for i:=0 to 3 do
                        begin
                          j:=P_O_S;
                          imp:= odd(Y_S);
                          port[$3DE]:= i;
                          k:=0;
                          repeat
                            if imp then
                              begin
                                S_S[i,k]:=memw[$b800:j];
                                j:=j+80-$2000;
                              end
                             else
                              begin
                                S_S[i,k]:=memw[$b800:j];
                                j:=j+$2000;
                              end;
                            imp:=not imp;
                            k:= succ(k);
                          until k>=8;
                        end;
                end;
          ega : begin
                  port[$3CE]:=4;
                  l:= 0;
                  repeat
                    port[$3CF]:= l;
                    k:=0;
                    j:=P_O_S;
                    repeat
                      S_S[l,k]:= mem[$A000:j]+mem[$A000:succ(j)] shl 8;
                      j:=j+80;
                      k:= succ(k);
                    until k>=8;
                    l:= l+1;
                  until l=4;
                end;
          her : begin
                  j:= Y_S shr 1 *80+X_S shr 3;
                  for i:=0 to 5 do
                   begin
                     for k:=0 to 3 do S_S[i,k]:=memw[$b000+k*$200:j];
                     j:=j+80;
                   end;
                end;
          tan : begin
                  j:= Y_S shr 2*160+X_S shr 1;
                  k:=0;
                  repeat
                    for i:=0 to 3 do
                      begin
                        S_S[k,i+k shl 2]:=memw[$b800+$200*i:j];
                        S_S[k+3,i+k shl 2]:=memw[$b800+$200*i:j+2];
                      end;
                    j:=j+160;
                    k:= succ(k);
                  until k=3;
                end;
  end; (*  case Gd   *)
  affput(2,Gd,X_S,Y_S,0,0);
end;

procedure pos_mouse(x,y : integer);
  var reg:registres;
begin
  if x>314*res then x:=314*res
           else if x<0 then x:=0;
  if y>199 then y:=199
           else if y<0 then y:=0;
  if (x=X_S) and (y=Y_S) then exit;
  if int_m then
     begin
       with reg do
         begin
           ax:=4;
           cx:=x;
           dx:=y;
         end;
       intr($33,reg);
     end;
  hide_mouse;
  X_S:=x;
  Y_S:=y;
  case Gd of
          ams : begin
                  P_O_S:=(Y_S shr 1)*80+X_S shr 3+(Y_S and 1)*$2000;
                end;
          (*cga : begin
                  P_O_S:=(Y_S shr 1)*80+X_S shr 2+(Y_S and 1)*$2000;
                end;*)
          ega : begin
                  P_O_S:=Y_S*80+X_S shr 3;
                end;
  end; (*  case Gd   *)
  show_mouse;
end;

procedure read_pos_mouse(var x,y,c:integer);
   var reg:registres;
begin
  if int_m then
     begin
       reg.ax:=3;
       intr($33,reg);
       x:=reg.cx;
       y:=reg.dx;
       c:=reg.bx;
     end
    else
     begin
       c:=0;
       x:=X_S;
       y:=Y_S;
     end;
end;

procedure mov_mouse(var funct : boolean; var key : char);
     var p_key : boolean;
         in1, in2 : char;
         x,y,cx,cy,cd:integer;
         reg:registres;
   begin
     if int_m then
     begin
       reg.ax:=3;
       intr($33,reg);
       x:=reg.cx;
       y:=reg.dx;
       cd:=reg.bx;
       pos_mouse(x,y);
       if cd<>0 then
          begin
            clic:=true;
            exit;
          end;
     end;
     funct:=false;
     key:=#255;
     p_key:=keypressed;
     while p_key do
        begin
          read(kbd,in1);
          read_pos_mouse(cx, cy, cd);
          case upcase(in1) of
                         '4' : cx:= cx - 8;
                         '2' : cy:= cy + 8;
                         '6' : cx:= cx + 8;
                         '8' : cy:= cy - 8;
                         '7' : begin
                                 cy:=1;
                                 cx:=1;
                               end;
                         '1' : begin
                                 cx:=1;
                                 cy:=190;
                               end;
                         '9' : begin
                                 cx:=315*res;
                                 cy:=1;
                               end;
                         '3' : begin
                                 cy:=190;
                                 cx:=315*res;
                               end;
                         '5' : begin
                                 cy:=100;
                                 cx:=155*res;
                               end;
                    ' ', #13 : begin
                                 clic:=true;
                                 exit;
                               end;
                         #27 : begin
                                 p_key:=keypressed;
                                 if p_key then
                                    begin
                                      read(kbd,in2);
                                      case in2 of
                                               'K' : cx:= pred(cx);
                                               'P' : cy:= succ(cy);
                                               'M' : cx:= cx + 2;
                                               'H' : cy:= pred(cy);
                                               ';'..'D' : begin
                                                            funct:= true;
                                                            key:=in2;
                                                            exit;
                                                          end;
                                               'G' : begin
                                                       cx:=cx-1;
                                                       cy:=cy-1;
                                                     end;
                                               'I' : begin
                                                       cx:=cx+1;
                                                       cy:=cy-1;
                                                     end;
                                               'O' : begin
                                                       cx:=cx-1;
                                                       cy:=cy+1;
                                                     end;
                                               'Q' : begin
                                                       cx:=cx+1;
                                                       cy:=cy+1;
                                                     end;
                                            end; (* case *)
                                    end;
                               end;
                         'I' : begin
                                 cx:= res*32;
                                 cy:= 8;
                               end;
                         'D' : begin
                                 cx:= 80*res;
                                 cy:= 8;
                               end;
                         'A' : begin
                                 cx:= 126*res;
                                 cy:= 8;
                               end;
                         'S' : begin
                                 cx:= 174*res;
                                 cy:= 8;
                               end;
                         'P' : begin
                                 cx:= 222*res;
                                 cy:= 8;
                               end;
                         'F' : begin
                                 cx:= res*270;
                                 cy:= 8;
                               end;
                         #19 : begin
                                 sonoff:= not sonoff;
                                 exit;
                               end;
                         #22 : begin
                                 zuul:=True;
                                 exit;
                               end;
                         #20 : begin           (* ^T => mode tandy *)
                                 funct:= true;
                                 key:= #9;
                               end;
                          #8 : begin           (* ^H => mode Hercule *)
                                 funct:= true;
                                 key:= #7;
                               end;
                    #1,#3,#5 : begin
                                 funct:= true;
                                 key:=in1;
                               end;
                end;
          pos_mouse(cx, cy);
          p_key:=keypressed;
        end;
   end;
