program mortevielle;

 {$v-}
 {$k-}
 {$x-}
 {$c-}

 {$i  var.mor   }         (* les variables *)
 {$i  as        }         (* assembleur *)
 {$i  keyboard  }         (* la gestion du clavier *)
 {$i  mouse     }         (* la souris  *)
 {$i  level15   }         (* les fonctions de base *)
 {$i  droite    }         (* pour faire une droite *)
 {$i  boite     }
 {$i  sprint    }         (* l'affichage des textes *)
 {$i  outtext   }         (* fonctions sup‚rieures d'affichage *)
 {$i  parole    }         (* les fonctions de la parole *)
 {$i  alert     }
 {$i  parole2   }         (* les fonctions de la parole *)
 (*  debug              le debugging *)
 {$i  ques      }         (* les questions *)
 {$i  menu      }         (* les fonctions du menu *)
 {$i  mor       }         (* divers *)
 {$i  taffich   }         (* chargement des dessins *)
 {$i  ovd1      }         (* les fonctions sur disque 1 *)
 {$i  mor2      }         (* divers *)
 {$i  actions   }         (* les actions *)
 {$i  prog      }         (* fonctions toplevel *)
 {$i  disk      }         (* chargements et sauvegardes *)

(*  ecrihexa            utilitaire *)

procedure divers(np:integer;b:boolean);
begin
  teskbd;
  repeat
    parole(np, 0, 0);
    atf3f8(key);
    if newGd<>Gd then
       begin
         Gd:=newGd;
         hirs;
         aff50(b);
       end;
  until key= 66;
end;

(* NIVEAU 0 *)

begin
(*init_debug;*)
(*  ecri_seg;*)
  Gd:=Cga;
  newGd:= Gd;
  zuul:= False;
  tesok:= False;
  chartex;
  charpal;
  charge_cfiph;
  charge_cfiec;
  zzuul(adcfiec+161,0,1644);
  c_zzz:= 1;
  init_nbrepm;
  init_mouse;
(*  crep:=memw[$7f00:0];
  memw[$7f00:0]:= crep+1;
  if (memw[$7f00:0]<> crep+1) then
     begin
       hirs;
       show_mouse;
       crep:=do_alert('[1][ 512 k  minimum requis  !!][OK]',1);
       clrscr;
       halt;
     end;
  if (Dseg+$1000>adbruit5) then
     begin
       hirs;
       show_mouse;
       crep:=do_alert('[1][ Il ne reste pas assez de |place m‚moire  !!][OK]',1);
       clrscr;
       halt;
     end;*)
  init_lieu;
  arret:= False;
  sonoff:= False;
  f2_all:= False;
  textcolor(9);
  teskbd;
  dialpre;
  newGd:= Gd;
  teskbd;
  if newGd<>Gd then Gd:=newGd;
  hirs;
  ades:= $7000;
  aff50(false);
  mlec:= 0;
  divers(142,False);

  ani50;
  divers(143,True);
  suite;
  music;
  adzon;
  takesav(0);
  if rech_cfiec then charge_cfiec;
  for crep:=1 to c_zzz do zzuul(adcfiec+161,0,1644);
  charge_bruit5;
  init_menu;

  theure;
  dprog;
  hirs;
  dessine_rouleau;
  show_mouse;
  repeat tjouer until arret;
  hide_mouse;
  clrscr;
(*out_debug;*)
end.

