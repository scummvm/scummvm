type str79 = string[79];

var le_file : text;

procedure init_debug;
begin
(* assign(le_file,'c:\usr\mort\temoin.mor');
 rewrite(le_file);*)
end;

procedure out_debug;
begin
(*  close(le_file);*)
end;

procedure debug(l : str79);
begin
  (*port[$3DD]:=15;
  gotoxy(1,10);
  write(l);
  writeln(le_file,l);*)
end;
