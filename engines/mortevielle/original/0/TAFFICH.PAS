procedure chardes( nom : str11; passe : long_integer; long : integer);
     var i, p, l : integer;
         b : byte;
         f : file;
   begin
     (* debug('chardes'); *)
     assign(f,nom);
     {$i-}
     reset(f);
     testfi;
     p:= 0;
     while passe>127 do
       begin
         p:=p+1;
         passe:=passe-128;
       end;
     if p<>0 then seek(f,p);
     p:= trunc(passe);
     l:= long+p;
     i:= 0;
     while l>0 do
       begin
         blockread(f,mem[$6000:i],1);
         testfi;
         l:= l-128;
         i:= i+128;
       end;
     close(f);
     for i:=p to long+p do mem[$7000:i-p]:=mem[$6000:i];
     {$i+}
   end;

procedure charani( nom : str11; passe : long_integer; long : integer);
     var i, p, l : integer;
         b : byte;
         f : file;
   begin
     (* debug('charani'); *)
     assign(f,nom);
     {$i-}
     reset(f);
     testfi;
     p:= 0;
     while passe>127 do
       begin
         passe:=passe-128;
         p:=p+1;
       end;
     if p<>0 then seek(f,p);
     p:= trunc(passe);
     l:=long+p;
     i:= 0;
     while l>0 do
       begin
         blockread(f,mem[$6000:i],1);
         testfi;
         l:=l-128;
         i:=i+128;
       end;
     close(f);
     for i:=p to long+p do mem[$7314:i-p]:=mem[$6000:i];
     {$i+}
   end;

procedure taffich;
   const tran1 : array[136..140] of byte
               = ( 121, 121, 138, 139, 120 );
         tran2 : array[153..161] of byte
               = ( 150, 150, 152, 152, 100,
                   110, 159, 100, 100 );
   var
      i, m, a, b, cx, handle,
      npal : integer;
      lgt, lhandle           : long_integer;
      nom                    : str11;
      PalH,k,j : integer;
      Alllum : array[0..15] of integer;

begin
 a:= caff;
 if a in [153..161] then a:= tran2[a]
                    else if a in [136..140] then a:= tran1[a];
 b:= a;
 if maff= a then exit;
 if a= 16 then
 begin
  s.pourc[9]:= '*';
  s.teauto[42]:= '*';
 end;
 if a= 20 then
 begin
  s.teauto[39]:= '*';
  if s.teauto[36]= '*' then
  begin
   s.pourc[3]:= '*';
   s.teauto[38]:= '*';
  end;
 end;
 if a= 24 then s.teauto[37]:= '*';
 if a= 30 then s.teauto[9]:= '*';
 if a= 31 then
 begin
  s.pourc[4]:= '*';
  s.teauto[35]:= '*';
 end;
 if a= 118 then s.teauto[41]:= '*';
 if a= 143 then s.pourc[1]:= '*';
 if a= 150 then s.teauto[34]:= '*';
 if a= 151 then s.pourc[2]:= '*';
 okdes:= true;
 hide_mouse;
 lgt:= 0;
 if ((a<>50) and (a<>51)) then
   begin
     m:= a+ 2000;
     if (m> 2001) and (m< 2010) then m:= 2001;
     if m= 2011 then m:= 2010;
     if a= 32 then m:= 2034;
     if (a= 17) and (maff= 14) then m:= 2018;
     if a> 99 then
       if (is= 1) or (is= 0) then m:= 2031 else m:= 2032;
     if ((a> 69) and (a< 80)) or (a= 30) or (a= 31) or (a= 144)
       or (a= 147) or (a= 149) then m:= 2030;
     if ((a< 27) and (((maff> 69) and (not s.ipre)) or (maff> 99)))
       or ((maff> 29) and (maff< 33)) then m:= 2033;
     messint(m);
     maff:= a;
     if a= 159 then a:= 86 else
      if a> 140 then a:= a- 67 else
       if a> 137 then a:= a- 66 else
        if a> 99 then a:= a- 64 else
         if a> 69 then a:= a- 42 else
          if a> 29 then a:= a- 5 else
           if a= 26 then a:= 24 else
            if a> 18 then a:= a- 1;
     npal:= a;
     for cx:= 0 to (a- 1) do lgt:= lgt+l[cx];
     handle:=l[a];
     nom:= 'DXX.mor';
   end
 else
   begin
     nom:= 'DZZ.mor';
     handle:= l[87];
     if a= 51 then
        begin
          lgt:= handle;
          handle:= l[88];
        end;
     maff:= a;
     npal:= a+37;
   end;
 chardes(nom,lgt,handle);
 if Gd=Her then
    begin
      for i:=0 to 15 do
         begin
           PalH:=memw[$7000:succ(i) shl 1];
           Alllum[i]:=PalH and 15 + PalH shr 12 and 15 + PalH shr 8 and 15;
         end;
      for i:=0 to 15 do
         begin
           k:=0;
           for j:=0 to 15 do if Alllum[j]>Alllum[k] then k:=j;
           mem[$7000:2+k shl 1]:= Rang[i];
           Alllum[k]:=-1;
         end;
    end;
 numpal:=npal;
 writepal(npal);

 if (b< 15) or (b= 16) or (b= 17) or (b= 24) or (b= 26) or (b= 50) then
    begin
      lgt:= 0;
      if (b< 15) or (b= 16) or (b= 17) or (b= 24) or (b= 26) then
         begin
           if b= 26 then b:= 18 else
              if b= 24 then b:= 17 else
                 if b> 15 then b:= b- 1;
           for cx:= 0 to (b- 1) do lgt:= lgt+ l[cx+89];
           handle:=l[b+89];
           nom:= 'AXX.mor';
         end
       else
         if b= 50 then
            begin
              nom:= 'AZZ.mor';
              handle:= 1260;
            end;
      charani(nom,lgt,handle);
    end;
 show_mouse;
 if (a< 27) and ((maff< 27) or (s.mlieu= 15)) and (msg[4]<> entrer) then
    begin
      if (a= 13) or (a= 14) then person
                            else if not blo then t11(s.mlieu, cx);
      mpers:=  0;
    end;
end;

(*    begin
      for i:=0 to 15 do
         begin
           PalH:=memw[$7000:succ(i) shl 1];
           Alllum[i]:=PalH and 15 + PalH shr 4 and 15 + PalH shr 8 and 15;
         end;
      for i:=0 to 15 do
         begin
           k:=0;
           for j:=0 to 15 do if Alllum[j]>Alllum[k] then k:=j;
           mem[$7000:2+k shl 1]:= Rang[i];
           Alllum[k]:=-1;
         end;
    end;*)