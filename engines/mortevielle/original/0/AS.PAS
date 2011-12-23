(*---------------------------------------------------------------------------*)
(*--------------------   PROCEDURES  EN  ASSEMBLEUR   -----------------------*)
(*---------------------------------------------------------------------------*)

procedure decomp(seg,dep:integer); external 'c:\mc\decomp.com';
(* procedure affams(seg,dep,x,y:integer); external 'c:\mc\affams.com';
 procedure affcga(seg,dep,x,y:integer); external 'c:\mc\cga.com';
 procedure affega(seg,dep,x,y:integer); external 'c:\mc\affega.com';
 procedure affher(seg,dep,x,y:integer); external 'c:\mc\affher.com';
 procedure afftan(seg,dep,x,y:integer); external 'c:\mc\afftan.com';*)
procedure afff(Gd,seg,dep,x,y:integer); external 'c:\mc\affich.com';
procedure S_sauv(Gd,y,dy:integer);  external 'c:\mc\sauvecr.com';
procedure S_char(Gd,y,dy:integer);  external 'c:\mc\charecr.com';
procedure musyc(tb:tablint;nbseg,att:integer);  external 'c:\mc\reusint.com';
procedure litph(t:tablint;typ,tempo:integer); external 'c:\mc\phint.com';
procedure demus(Src, Dst, Tay : integer); external 'c:\mc\demus.com';
procedure zzuul(ad,seg,tai:integer); external 'c:\mc\zuul.com';
