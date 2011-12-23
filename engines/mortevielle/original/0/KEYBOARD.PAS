function readkey : char;
   var c : char;
  begin
    read(kbd,c);
    readkey:= c;
  end;

function testou: integer;
    var ch:char;
  begin
    read(kbd,ch);
    case  ch  of
              #19 : sonoff:= not sonoff;
              #1,
              #3,
              #5  : begin
                      newGd:= pred(ord(ch)) shr 1;
                      testou:= 61;
                      exit;
                    end;
              #8  : begin
                      newGd:= Her;
                      testou:= 61;
                      exit;
                    end;
              #20 : begin
                      newGd:= Tan;
                      testou:= 61;
                      exit;
                    end;
              #22 : if (c_zzz in [1,2]) then
                       begin
                         zzuul(adcfiec+161,0,1644);
                         c_zzz:=succ(c_zzz);
                         testou:=61;
                         exit;
                       end;
              #27 : if keypressed then read(kbd,ch);
      end;
    testou:=ord(ch);
  end;

procedure teskbd;
    var dum:integer;
  begin
    if keypressed then dum:=testou;
  end;

