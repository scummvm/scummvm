void fill_box(integer x,integer y,integer dx)
{
  box(15,gd,pred(integer,x) << 3, pred(integer,y) << 3, pred(integer,x+dx) << 3, y << 3, 255);
}

void clear_box(integer x,integer y,integer dx)
{
  box(0,gd,pred(integer,x) << 3, pred(integer,y) << 3, pred(integer,x+dx) << 3, y << 3, 255);
}

void writeg(str255 l,integer c)

{
     integer i, x, xo, yo;
     integer cecr;
     boolean t;

/*  debug('writeg : '+l);*/

  if (l=="")  return;
  hide_mouse();
  xo=xwhere;
  yo=ywhere;
  if (res==2)  i=6;
           else i=10;
  x=xo+i*length(l);
  switch (c) {
          case 1:case 3  : {
                   cecr=0;
                   box(15,gd,xo,yo,x,yo+7,255);
                 }
                 break;
             case 4 : {
                   cecr=0;
                 }
                 break;
             case 5 : {
                   cecr=15;
                 }
                 break;
           case 0:case 2 : {
                   cecr=15;
                   box(0,gd,xo,yo,x,yo+7,255);
                 }
                 break;
  }
  xo=xo+1;
  yo=yo+1;
  for( x=1; x <= length(l); x ++)
     {
       affcar(gd,xo,yo,cecr,ord(l[x]));
       xo=xo+i;
     }
  show_mouse();
}
