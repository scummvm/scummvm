var delig : str255;
const
               tabdr : array [0..31] of byte
                     = (  32, 101, 115,  97, 114, 105, 110,
                         117, 116, 111, 108,  13, 100,  99,
                         112, 109,  46, 118, 130,  39, 102,
                          98,  44, 113, 104, 103,  33,  76,
                          85, 106,  30,  31 );


               tab30 : array [0..31] of byte
                     =(   69,  67,  74, 138, 133, 120,  77, 122,
                         121,  68,  65,  63,  73,  80,  83,  82,
                         156,  45,  58,  79,  49,  86,  78,  84,
                          71,  81,  64,  66, 135,  34, 136,  91 );



               tab31 : array [0..31] of byte
                     = (  93,  47,  48,  53,  50,  70, 124,  75,
                          72, 147, 140, 150, 151,  57,  56,  51,
                         107, 139,  55,  89, 131,  37,  54,  88,
                         119,   0,   0,   0,   0,   0,   0,   0 );

procedure DeLine( num :integer ; var L:phrase ; var TL:integer );
        var i,j,TS : integer;
            let : char;
             pS, k : byte;
            the_end : boolean;
        procedure cinq_huit( var c:char; var ind:integer; var pt:byte);
             const rap : array[0..31] of char
                 =( ',', ':', '@', '!', '?', '-', '‡',
                    ' ', '…', 'ƒ', '‚', 'Š', 'ˆ', '/',
                    '‹', 'Œ', '¢', #39, '“', '"', '—',
                    '–', '0', '1', '2', '3', '4', '5',
                    '6', '7', '8', '9');
             var oct,ocd:integer;

           begin    (* 5-8 *)
             oct:=T_mot[ind];
             oct:=(oct shl (16-pt)) shr (16-pt);
             if pt<6 then
                begin
                  ind:=ind+1;
                  oct:=oct shl (5-pt);
                  pt:=pt+11;
                  oct:=oct or (T_mot[ind] shr pt);
                end
              else
                begin
                  pt:=pt-5;
                  oct:=oct shr pt;
                end;

             case oct of
                          11 : begin
                                 c:= '$';
                                 The_end:= True;
                               end;
                       30,31 : begin
                                 ocd:=T_mot[ind];
                                 ocd:=(ocd shl (16-pt)) shr (16-pt);
                                 if pt<6 then
                                    begin
                                      ind:=ind+1;
                                      ocd:=ocd shl (5-pt);
                                      pt:=pt+11;
                                      ocd:=ocd or (T_mot[ind] shr pt);
                                    end
                                  else
                                    begin
                                      pt:=pt-5;
                                      ocd:=ocd shr pt;
                                    end;
                                  if oct=30 then c:=chr(tab30[ocd])
                                            else c:=chr(tab31[ocd]);
                                  if c=#0 then
                                     begin
                                       The_end:=True;
                                       c:='#';
                                     end;
                                end;
                      else c:=chr(tabdr[oct]);
                    end;
           end;   (* 5-8 *)

      begin   (* DETEX *)
        (*debug('  => DeLine');*)
        delig:='';
        TS:= T_rec[num].indis;
        pS:= T_rec[num].point;
        i:=TS;
        TL:=1;
        j:=1;
        k:=pS;
        the_end:=False;
        repeat
          cinq_huit(let,i,k);
          L[j]:=let;
          if j<254 then delig:=delig+let;
          j:=j+1;
        until  the_end;
        TL:=j-1;
        if TL<255 then delig:=copy(delig,1,TL-1);    (* enleve le $ *)
      end;    (* DETEX *)


procedure afftex( Ch: phrase; x,y,dx,dy, typ : integer);
    var the_end : boolean;
        touch : char;
        xf,yf : integer;
        xc,yc : integer;
        tab, p : integer;
        s : string[255];
        i,j,nt : integer;

    function l_motsuiv(p:integer):integer;
        var c:integer;
      begin
        c:=p;
        while not (Ch[p] in [' ','$','@']) do p:=p+1;
        l_motsuiv:=tab*(p-c);
      end;

  begin
(*    debug('  .. Afftex');*)
    putxy(x,y);
    if res=1 then tab:=10 else tab:=6;
    dx:= dx * 6;
    dy:= dy * 6;
    xc:= x;
    yc:= y;
    xf:= x+dx;
    yf:= y+dy;
    p:= 1;
    the_end:=(Ch[p]='$');
    s:='';
    while not the_end do
     begin
      case Ch[p] of
                  '@' : begin
                          writeg(s,typ);
                          s:='';
                          p:=p+1;
                          xc:=x;
                          yc:=yc+6;
                          putxy(xc,yc);
                        end;
                  ' ' : begin
                          s:=s+' ';
                          xc:=xc+tab;
                          p:=p+1;
                          if l_motsuiv(p)+xc > xf then
                             begin
                               writeg(s,typ);
                               s:='';
                               xc:=x;
                               yc:=yc+6;
                               if yc>yf then
                                  begin
                                    repeat until keypressed;
                                    i:=y;
                                    repeat
                                      j:=x;
                                      repeat
                                        putxy(j,i);
                                        writeg(' ',0);
                                        j:=j+6;
                                      until j>xf;
                                      i:=i+6;
                                    until i>yf;
                                    yc:=y;
                                  end;
                               putxy(xc,yc);
                             end;
                        end;
                  '$' : begin
                          the_end:= True;
                          writeg(s,typ);
                        end;
        else
        begin
          s:=s+Ch[p];
          p:=p+1;
          xc:=xc+tab;
        end;
      end;  (* case *)
    end;
 end;
