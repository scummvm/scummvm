
/* overlay */ boolean ques()
{
         const array<1,10,integer> ta
             = {{ 511, 516, 524, 531, 545,
                 552, 559, 563, 570, 576}};
         const array<1,10,integer> ok
            = {{ 4, 7, 1, 6, 4, 4, 2, 5, 3, 1 }};

       boolean q, func, test;
       integer i,j,k,y,memk;
       integer tay , tmax;
       integer rep, prem, der;
       phrase st;
       char key;
       mult_rect coor;
       array<1,14,str40> chaines;
       integer compte;


  boolean ques_result;
  test=false;
  i=0;
  compte= 0;

  do {
    hide_mouse();
    hirs();
    show_mouse();
    i= i+1;
    deline(ta[i],st,tay);
    if (res==1)  y=29; else y=23;
    box(15,gd,0,14,630,y,255);
    afftex(st,20,15,100,2,0);
    if (i!=10) 
       {
         prem= ta[i]+1;
         der= ta[i+1]-1;
       }
     else
       {
         prem= 503;
         der= 510;
       }
    y=35;
    tmax= 0;
    memk= 1;
    for( j= prem; j <= der; j ++)
      {
        deline(j,st,tay);
        if (tay>tmax)  tmax= tay;
        afftex(st,100,y,100,1,0);
        chaines[memk]= delig;
        memk= memk+1;
        y= y+8;
      }
    for( j= 1; j <= succ(integer,der-prem); j ++)
      {
      {
          rectangle& with = coor[j]; 

          with.x1= 45*res;
          with.x2= (tmax*3 + 55)*res;
          with.y1= 27 + j*8;
          with.y2= 34 + j*8;
          with.etat= true;
      }
        while (length(chaines[j] )<tmax) 
          {
            chaines[j]= chaines[j] +' ';
          }
      }
    coor[j+1].etat= false;
    if (res==1)  rep=10; else rep=6;
    boite(80, 33, 40+tmax*rep, (der-prem)*8+ 16, 15);
    rep= 0;
    j= 0;
    memk= 0;
    do {
      clic= false;
      tesok=false;
      mov_mouse(func,key);
      k= 1;
      while (coor[k].etat && ! dans_rect(coor[k]))  k= k+1;
      if (coor[k].etat) 
         {
           if ((memk!=0) && (memk!=k)) 
              {
                /*DeLine(T_rec[ta[i]+memk].indis,T_rec[ta[i]+memk].point,st,tay);*/
                for( j= 1; j <= tmax; j ++) st[j]= chaines[memk][j];
                st[1+tmax]= '$';
                afftex(st,100,27+memk*8,100,1,0);
              }
           if (memk!=k) 
              {
                /*DeLine(T_rec[pred(prem)+k].indis,T_rec[pred(prem)+k].point,st,tay);*/
                for( j= 1; j <= tmax; j ++) st[j]= chaines[k][j];
                st[1+tmax]= '$';
                afftex(st,100,27+k*8,100,1,1);
                memk= k;
              }
         }
       else
         if (memk!=0) 
            {
              for( j= 1; j <= tmax; j ++) st[j]= chaines[memk][j];
              st[1+tmax]= '$';
              afftex(st,100,27+memk*8,100,1,0);
              memk=0;
            }
    } while (!((memk!=0) && clic));
    if (memk==ok[i])  compte=compte+1;
                  else {
                         if (i==5)  i=i +1;
                         if ((i==7) || (i==8))  i= 10;
                       }
    if (i==10)  q=   /*testprot*/ true;
  } while (!(i==10));
  ques_result= (compte==10) && q;
  return ques_result;
}
