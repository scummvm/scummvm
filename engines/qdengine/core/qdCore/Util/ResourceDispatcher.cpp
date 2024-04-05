
#include "qd_precomp.h"

#include "ResourceDispatcher.h"

int ResourceUser::IDs;

void ResourceDispatcher::do_start()
{
	if(start_log){
		start_log = false;
		syncro_timer.setTime(1);
		for(UserList::iterator i = users.begin(); i != users.end(); ++i)
			(*i) -> time = syncro_timer();
	}
}

void ResourceDispatcher::reset()
{
	for(UserList::iterator i = users.begin(); i != users.end(); ++i)
		(*i) -> time = syncro_timer();
}

void ResourceDispatcher::quant()
{
	if(users.empty())
		return;

	do_start();
	
	syncro_timer.next_frame();

	for(;;) {
		time_type t_min = users.front() -> time;
		ResourceUser* user_min = users.front();
		for(UserList::iterator i = users.begin(); i != users.end(); ++i){
			ResourceUser& u = **i;
			if(t_min > u.time){
				t_min = u.time;
				user_min = &u;
				}
			}
		if(t_min < syncro_timer()){
			if(!user_min -> quant())
				detach(user_min);
			else
				user_min -> time += user_min -> time_step();
			}
		else
			break;
		} 
}	
