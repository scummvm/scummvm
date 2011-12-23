const integer m_arrow = 0;
const integer m_point_hand = 1;

boolean int_m,
clic;

integer m_show,
x_s,
y_s,
p_o_s,
mouse_shwn;

matrix<0,5,0,11,integer> s_s;


void init_mouse()
{
      integer i, j;
       registres reg;

  mouse_shwn= 0;
  x_s= 0;
  y_s= 0;
  p_o_s= 0;
  /*int_m:= False;*/
  clic= false;
  m_show= m_arrow;
  if ((memw[0xcc]==0) && (memw[0xce]==0))  int_m= false;
  if (int_m) 
   {
    reg.ax= 0;
    intr(0x33,reg);
    int_m= (reg.ax==-1);
    if (int_m) 
      {
         {
           reg.ax=4;
           reg.cx=0;
           reg.dx=0;
         }
       intr(0x33,reg);
     }
   }
}

/*procedure change_mouse( t : integer );
begin
  m_show:= t;
end;*/

/*procedure dessine_souris(xf,yf:integer);
  var x, xx, y :integer;
begin
  case m_show of
             m_arrow : begin
                         affput(2,Gd,xf,yf,0,0);
                       end;
  end;
end;  */

void hide_mouse()
{
      integer i,j,k,ps,ds;
      boolean imp;

  mouse_shwn = pred(integer,mouse_shwn);
  if (mouse_shwn==0) 
     {
       imp= odd(y_s);
       j=p_o_s;
       switch (gd) {
             case cga : {
                     k=0;
                     j=((cardinal)y_s >> 1) * 80+ ((cardinal)x_s >> 2);
                     do {
                       memw[0xb000+j]=s_s[0][k];
                       memw[0xb800+j+2]=s_s[1][k];
                       memw[0xba00+j]=s_s[2][k];
                       memw[0xba00+j+2]=s_s[3][k];
                       j=j+80;
                       k= succ(integer,k);
                     } while (!(k>=5));
                   }
                   break;
                case ams : {
                        for( i=0; i <= 3; i ++)
                        {
                          port[0x3dd]= 1 << i;
                          k=0;
                          j=p_o_s;
                          do {
                            if (imp) 
                              {
                                memw[0xb800+j]=s_s[i][k];
                                j=j+80-0x2000;
                              }
                             else
                              {
                                memw[0xb800+j]=s_s[i][k];
                                j=j+0x2000;
                              }
                            imp=! imp;
                            k= succ(integer,k);
                          } while (!(k>=8));
                        }
                      }
                      break;
                case ega : {
                        port[0x3c4]=2;
                        port[0x3ce]=8;
                        port[0x3cf]=255;
                        i= 0;
                        do {
                          port[0x3c5]= 1 << i;
                          k=0;
                          j=p_o_s;
                          do {
                            ps=mem[0xa000+j];
                            mem[0xa000+j]=lo(s_s[i][k]);
                            ps=mem[0xa000+j+1];
                            mem[0xa000+j+1]=hi(s_s[i][k]);
                            j=j+80;
                            k= succ(integer,k);
                          } while (!(k>=8));
                          i= i+1;
                        } while (!(i==4));
                      }
                      break;
                case her : {
                        j= ((cardinal)y_s >> 1) *80+((cardinal)x_s >> 3);
                        for( i=0; i <= 5; i ++)
                         {
                           for( k=0; k <= 3; k ++) memw[0xb000+k*0x200+j]=s_s[i][k];
                           j=j+80;
                         }
                      }
                      break;
                case tan : {
                        j= ((cardinal)y_s >> 2) *160+((cardinal)x_s >> 1);
                        k=0;
                        do {
                          for( i=0; i <= 3; i ++)
                           {
                             memw[0xb800+0x200*i+j]=s_s[k][i+(k << 2)];
                             memw[0xb800+0x200*i+j+2]=s_s[k+3][i+(k << 2)];
                           }
                          j=j+160;
                          k= succ(integer,k);
                        } while (!(k==3));
                      }
                      break;

       }     /* case Gd */
     }
}

void show_mouse()
{
      integer i, j, k, l,
      ps,ds;
      boolean imp;
      char c;

  mouse_shwn = mouse_shwn +1;
  if (mouse_shwn!=1)  return;
  j=p_o_s;
  imp= odd(y_s);
  i=x_s & 7;
  switch (gd) {
             case cga : {
                     k=0;
                     j=((cardinal)y_s >> 1) * 80+ ((cardinal)x_s >> 2);
                     do {
                       s_s[0][k]=memw[0xb800+j];
                       s_s[1][k]=memw[0xb800+j+2];
                       s_s[2][k]=memw[0xba00+j];
                       s_s[3][k]=memw[0xba00+j+2];
                       j=j+80;
                       k= succ(integer,k);
                     } while (!(k>=5));
                   }
                   break;
          case ams : {
                  for( i=0; i <= 3; i ++)
                        {
                          j=p_o_s;
                          imp= odd(y_s);
                          port[0x3de]= i;
                          k=0;
                          do {
                            if (imp) 
                              {
                                s_s[i][k]=memw[0xb800+j];
                                j=j+80-0x2000;
                              }
                             else
                              {
                                s_s[i][k]=memw[0xb800+j];
                                j=j+0x2000;
                              }
                            imp=! imp;
                            k= succ(integer,k);
                          } while (!(k>=8));
                        }
                }
                break;
          case ega : {
                  port[0x3ce]=4;
                  l= 0;
                  do {
                    port[0x3cf]= l;
                    k=0;
                    j=p_o_s;
                    do {
                      s_s[l][k]= mem[0xa000+j]+(mem[0xa000+succ(integer,j)] << 8);
                      j=j+80;
                      k= succ(integer,k);
                    } while (!(k>=8));
                    l= l+1;
                  } while (!(l==4));
                }
                break;
          case her : {
                  j= ((cardinal)y_s >> 1) *80+((cardinal)x_s >> 3);
                  for( i=0; i <= 5; i ++)
                   {
                     for( k=0; k <= 3; k ++) s_s[i][k]=memw[0xb000+k*0x200+j];
                     j=j+80;
                   }
                }
                break;
          case tan : {
                  j= ((cardinal)y_s >> 2)*160+((cardinal)x_s >> 1);
                  k=0;
                  do {
                    for( i=0; i <= 3; i ++)
                      {
                        s_s[k][i+(k << 2)]=memw[0xb800+0x200*i+j];
                        s_s[k+3][i+(k << 2)]=memw[0xb800+0x200*i+j+2];
                      }
                    j=j+160;
                    k= succ(integer,k);
                  } while (!(k==3));
                }
                break;
  }    /*  case Gd   */
  affput(2,gd,x_s,y_s,0,0);
}

void pos_mouse(integer x,integer y)
{
      registres reg;

  if (x>314*res)  x=314*res;
           else if (x<0)  x=0;
  if (y>199)  y=199;
           else if (y<0)  y=0;
  if ((x==x_s) && (y==y_s))  return;
  if (int_m) 
     {
         {
           reg.ax=4;
           reg.cx=x;
           reg.dx=y;
         }
       intr(0x33,reg);
     }
  hide_mouse();
  x_s=x;
  y_s=y;
  switch (gd) {
          case ams : {
                  p_o_s=((cardinal)y_s >> 1)*80+((cardinal)x_s >> 3)+(y_s & 1)*0x2000;
                }
                break;
          /*cga : begin
                  P_O_S:=(Y_S shr 1)*80+X_S shr 2+(Y_S and 1)*$2000;
                end;*/
          case ega : {
                  p_o_s=y_s*80+((cardinal)x_s >> 3);
                }
                break;
  }    /*  case Gd   */
  show_mouse();
}

void read_pos_mouse(integer& x,integer& y,integer& c)
{
       registres reg;

  if (int_m) 
     {
       reg.ax=3;
       intr(0x33,reg);
       x=reg.cx;
       y=reg.dx;
       c=reg.bx;
     }
    else
     {
       c=0;
       x=x_s;
       y=y_s;
     }
}

void mov_mouse(boolean& funct, char& key)
   {
         boolean p_key;
         char in1, in2;
         integer x,y,cx,cy,cd;
         registres reg;

     if (int_m) 
     {
       reg.ax=3;
       intr(0x33,reg);
       x=reg.cx;
       y=reg.dx;
       cd=reg.bx;
       pos_mouse(x,y);
       if (cd!=0) 
          {
            clic=true;
            return;
          }
     }
     funct=false;
     key='\377';
     p_key=keypressed();
     while (p_key) 
        {
          input >> kbd >> in1;
          read_pos_mouse(cx, cy, cd);
          switch (upcase(in1)) {
                         case '4' : cx= cx - 8; break;
                         case '2' : cy= cy + 8; break;
                         case '6' : cx= cx + 8; break;
                         case '8' : cy= cy - 8; break;
                         case '7' : {
                                 cy=1;
                                 cx=1;
                               }
                               break;
                         case '1' : {
                                 cx=1;
                                 cy=190;
                               }
                               break;
                         case '9' : {
                                 cx=315*res;
                                 cy=1;
                               }
                               break;
                         case '3' : {
                                 cy=190;
                                 cx=315*res;
                               }
                               break;
                         case '5' : {
                                 cy=100;
                                 cx=155*res;
                               }
                               break;
                    case ' ': case '\15' : {
                                 clic=true;
                                 return;
                               }
                               break;
                         case '\33' : {
                                 p_key=keypressed();
                                 if (p_key) 
                                    {
                                      input >> kbd >> in2;
                                      switch (in2) {
                                               case 'K' : cx= pred(integer,cx); break;
                                               case 'P' : cy= succ(integer,cy); break;
                                               case 'M' : cx= cx + 2; break;
                                               case 'H' : cy= pred(integer,cy); break;
                                               case RANGE_10(';','D') : {
                                                            funct= true;
                                                            key=in2;
                                                            return;
                                                          }
                                                          break;
                                               case 'G' : {
                                                       cx=cx-1;
                                                       cy=cy-1;
                                                     }
                                                     break;
                                               case 'I' : {
                                                       cx=cx+1;
                                                       cy=cy-1;
                                                     }
                                                     break;
                                               case 'O' : {
                                                       cx=cx-1;
                                                       cy=cy+1;
                                                     }
                                                     break;
                                               case 'Q' : {
                                                       cx=cx+1;
                                                       cy=cy+1;
                                                     }
                                                     break;
                                            }    /* case */
                                    }
                               }
                               break;
                         case 'I' : {
                                 cx= res*32;
                                 cy= 8;
                               }
                               break;
                         case 'D' : {
                                 cx= 80*res;
                                 cy= 8;
                               }
                               break;
                         case 'A' : {
                                 cx= 126*res;
                                 cy= 8;
                               }
                               break;
                         case 'S' : {
                                 cx= 174*res;
                                 cy= 8;
                               }
                               break;
                         case 'P' : {
                                 cx= 222*res;
                                 cy= 8;
                               }
                               break;
                         case 'F' : {
                                 cx= res*270;
                                 cy= 8;
                               }
                               break;
                         case '\23' : {
                                 sonoff= ! sonoff;
                                 return;
                               }
                               break;
                         case '\26' : {
                                 zuul=true;
                                 return;
                               }
                               break;
                         case '\24' : {           /* ^T => mode tandy */
                                 funct= true;
                                 key= '\11';
                               }
                               break;
                          case '\10' : {           /* ^H => mode Hercule */
                                 funct= true;
                                 key= '\7';
                               }
                               break;
                    case '\1':case '\3':case '\5' : {
                                 funct= true;
                                 key=in1;
                               }
                               break;
                }
          pos_mouse(cx, cy);
          p_key=keypressed();
        }
   }
