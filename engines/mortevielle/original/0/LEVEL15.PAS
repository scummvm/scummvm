
(* NIVEAU 15 *)
procedure copcha;
  var i:integer;
begin
  i:= acha;
  repeat
    tabdon[i]:=tabdon[i+390];
    i:=succ(i);
  until i=acha +390;
end;

function Dans_rect( R : rectangle ): boolean;
     var x, y, c : integer;
   begin
     read_pos_mouse(x,y,c);
     with R do
       if (x>x1) and
          (x<x2) and
          (y>y1) and
          (y<y2) then Dans_rect:= True
                 else Dans_rect:= False;
   end;

procedure OutBloc(n : integer; p : pattern; pal:t_nhom);
     var i,j,ad : integer;
   begin
     ad:=n*404+$D700;
     with p do
       begin
         memw[$6000:ad]:= tax;
         memw[$6000:ad+2]:= tay;
         ad:=ad+4;
         for i:=1 to tax do
           for j:=1 to tay do
             mem[$6000:ad+pred(j)*tax+pred(i)]:= pal[n].hom[des[i,j]];
       end;
   end;


procedure writepal(n:integer);
   var   i : integer;
       pal : t_nhom;

 begin
   case Gd of
           tan,
           ega,
           ams : for i:=1 to 16 do
                   begin
                     mem[$7000:2*i]:= Tabpal[n,i].x;
                     mem[$7000:succ(2*i)]:= Tabpal[n,i].y;
                   end;
           cga : begin
                   pal:= palcga[n].a;
                   if (n<89) then palette(palcga[n].p);
                   for i:=0 to 15 do OutBloc(i, tpt[Pal[i].n],pal);
                 end;
   end;
 end;


procedure PictOut(seg,dep,x,y:integer);
   begin
     decomp(seg,dep);
     if Gd=Her then
        begin
          mem[$7000:2]:=0;
          mem[$7000:32]:=15;
        end;
     if (caff<>51) and (memw[$7000:$4138]>$100) then memw[$7000:$4138]:=$100;
     afff(Gd,seg,dep,x,y);
   end;

procedure putxy(x,y:integer);
begin
  xwhere:=x;
  ywhere:=y;
end;
(*
function calad(x,y:integer):integer;
   begin
     calad:=pred(x)+320*pred(y)
   end;
  *)
procedure sauvecr(y,dy:integer);
  begin
    hide_mouse;
    S_sauv(Gd,y,dy);
    show_mouse;
  end;

procedure charecr(y,dy:integer);
  begin
    hide_mouse;
    S_char(Gd,y,dy);
    show_mouse;
  end;
(*
function peek(ad:integer):integer;
begin
  peek:=tabdon[ad];
end;

function peekw(ad:integer):integer;
begin
  peekw:=tabdon[ad] shl 8+tabdon[succ(ad)];
end;

function peekl(ad:integer):real;
begin
  peekl:=tabdon[ad+3]+tabdon[ad+2] shl 8;
end;


procedure poke (ad,n:integer);
begin
  tabdon[ad]:=lo(n);
end;
  *)

procedure adzon;
     var f:file;
         g:file of byte;
         a:byte;
         i:integer;
   begin
     assign(f,'don.mor');
     reset(f,256);
     blockread(f,tabdon,7);
     close(f);
     assign(f,'bmor.mor');
     reset(f,1916);
     blockread(f,tabdon[fleche],1);
     (*i:=fleche;
     repeat
       read(g,a);
       tabdon[i]:=a;
       i:=succ(i);
     until i=fleche + 1916;*)
     close(f);
     assign(f,'dec.mor');
     reset(f,1664);
     blockread(f,mem[$73A2:0],1);
     (*i:=0;
     repeat
       read(g,a);
       mem[$73A2:i]:=a;
       i:=succ(i);
     until eof(g);*)
     close(f);
   end;

function animof (ouf,num:integer):integer;
     var nani, aux : integer;
   begin
     nani:= mem[adani:1];
     aux:= num;
     if ouf<>1 then aux:= aux+nani;
     animof:=nani shl 2 + 2+swap(memw[adani: aux shl 1]);
     (*aux:= nani shl 2 + 2;
     if ouf=1 then aux:= aux+ swap(memw[adani: num shl 1])
              else aux:= aux+ swap(memw[adani: (nani+num) shl 1]);
     animof:=aux;*)
   end;

(*procedure affgd(offs,dep,x,y:integer);
begin
  case Gd of
     Ams : affams(offs,dep,x,y);
     Cga : affcga(offs,dep,x,y);
     Ega : affega(offs,dep,x,y);
     Her : affher(offs,dep,x,y);
     Tan : afftan(offs,dep,x,y);
  end;
end;*)