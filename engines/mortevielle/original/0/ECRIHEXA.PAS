procedure ecri1hexa(y:integer);
begin
  if y in [0..9] then write(y)
                 else write(chr(y+55));
end;


procedure ecri_int(x:integer);
begin
  ecri1hexa(x shr 12);
  ecri1hexa((x shr 8) mod 16);
  ecri1hexa((x shr 4) mod 16);
  ecri1hexa(x mod 16);
end;

procedure ecri_seg;
  var c:char;
begin
  write('Segment programme => $');
  ecri_int(Cseg);
  writeln;
  write('Segment donnees   => $');
  ecri_int(Dseg);
  writeln;
  write('Segment pile      => $');
  ecri_int(Sseg);
  writeln;
  read(kbd,c);
end;
