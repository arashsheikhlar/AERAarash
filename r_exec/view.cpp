//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/
//_/_/ HUMANOBS - Replicode r_exec
//_/_/
//_/_/ Eric Nivel
//_/_/ Center for Analysis and Design of Intelligent Agents
//_/_/   Reykjavik University, Menntavegur 1, 101 Reykjavik, Iceland
//_/_/   http://cadia.ru.is
//_/_/ Copyright(c)2012
//_/_/
//_/_/ This software was developed by the above copyright holder as part of 
//_/_/ the HUMANOBS EU research project, in collaboration with the 
//_/_/ following parties:
//_/_/ 
//_/_/ Autonomous Systems Laboratory
//_/_/   Technical University of Madrid, Spain
//_/_/   http://www.aslab.org/
//_/_/
//_/_/ Communicative Machines
//_/_/   Edinburgh, United Kingdom
//_/_/   http://www.cmlabs.com/
//_/_/
//_/_/ Istituto Dalle Molle di Studi sull'Intelligenza Artificiale
//_/_/   University of Lugano and SUPSI, Switzerland
//_/_/   http://www.idsia.ch/
//_/_/
//_/_/ Institute of Cognitive Sciences and Technologies
//_/_/   Consiglio Nazionale delle Ricerche, Italy
//_/_/   http://www.istc.cnr.it/
//_/_/
//_/_/ Dipartimento di Ingegneria Informatica
//_/_/   University of Palermo, Italy
//_/_/   http://roboticslab.dinfo.unipa.it/index.php/Main/HomePage
//_/_/
//_/_/
//_/_/ --- HUMANOBS Open-Source BSD License, with CADIA Clause v 1.0 ---
//_/_/
//_/_/ Redistribution and use in source and binary forms, with or without 
//_/_/ modification, is permitted provided that the following conditions 
//_/_/ are met:
//_/_/
//_/_/ - Redistributions of source code must retain the above copyright 
//_/_/ and collaboration notice, this list of conditions and the 
//_/_/ following disclaimer.
//_/_/
//_/_/ - Redistributions in binary form must reproduce the above copyright 
//_/_/ notice, this list of conditions and the following
//_/_/ disclaimer in the documentation and/or other materials provided 
//_/_/ with the distribution.
//_/_/
//_/_/ - Neither the name of its copyright holders nor the names of its 
//_/_/ contributors may be used to endorse or promote products 
//_/_/ derived from this software without specific prior written permission.
//_/_/
//_/_/ - CADIA Clause: The license granted in and to the software under this 
//_/_/ agreement is a limited-use license. The software may not be used in 
//_/_/ furtherance of: 
//_/_/ (i) intentionally causing bodily injury or severe emotional distress 
//_/_/ to any person; 
//_/_/ (ii) invading the personal privacy or violating the human rights of 
//_/_/ any person; or 
//_/_/ (iii) committing or preparing for any act of war.
//_/_/
//_/_/ THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//_/_/ "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
//_/_/ LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
//_/_/ A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
//_/_/ OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
//_/_/ SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
//_/_/ LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
//_/_/ DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
//_/_/ THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
//_/_/ (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
//_/_/ OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//_/_/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

#include	"view.h"
#include	"../../CoreLibrary/trunk/CoreLibrary/utils.h"
#include	"group.h"
#include	"mem.h"

#include <limits>

const	float32	PLUS_INFINITY=std::numeric_limits<float>::infinity();


namespace	r_exec{

	CriticalSection	OIDCS;

	uint32	View::LastOID=0;

	uint32	View::GetOID(){

		OIDCS.enter();
		uint32	oid=LastOID++;
		OIDCS.leave();
		return	oid;
	}

	uint16	View::ViewOpcode;

	float32	View::MorphValue(float32	value,float32	source_thr,float32	destination_thr){

		if(value==0)
			return	destination_thr;
		
		if(source_thr>0){

			if(destination_thr>0){

				float32	r=value*destination_thr/source_thr;
				if(r>1)	//	handles precision errors.
					r=1;
				return	r;
			}else
				return	value;
		}
		return	destination_thr+value;
	}

	float32	View::MorphChange(float32	change,float32	source_thr,float32	destination_thr){	//	change is always >0.

		if(source_thr>0){

			if(destination_thr>0)
				return	change*destination_thr/source_thr;
			else
				return	change;
		}
		return	destination_thr+change;
	}

	View::View(View	*view,Group	*group):r_code::View(),controller(NULL){

		Group	*source=view->get_host();
		object=view->object;
		memcpy(_code,view->_code,VIEW_CODE_MAX_SIZE*sizeof(Atom));
		_code[VIEW_OID].atom=GetOID();
		references[0]=group;	//	host.
		references[1]=source;	//	origin.

		//	morph ctrl values; NB: res is not morphed as it is expressed as a multiple of the upr.
		code(VIEW_SLN)=Atom::Float(MorphValue(view->code(VIEW_SLN).asFloat(),source->get_sln_thr(),group->get_sln_thr()));
		switch(object->code(0).getDescriptor()){
		case	Atom::GROUP:
			code(GRP_VIEW_VIS)=Atom::Float(MorphValue(view->code(GRP_VIEW_VIS).asFloat(),source->get_vis_thr(),group->get_vis_thr()));
			break;
		case	Atom::NULL_PROGRAM:
		case	Atom::INSTANTIATED_PROGRAM:
		case	Atom::INSTANTIATED_CPP_PROGRAM:
		case	Atom::INSTANTIATED_INPUT_LESS_PROGRAM:
		case	Atom::INSTANTIATED_ANTI_PROGRAM:
		case	Atom::COMPOSITE_STATE:
		case	Atom::MODEL:
			code(VIEW_ACT)=Atom::Float(MorphValue(view->code(VIEW_ACT).asFloat(),source->get_act_thr(),group->get_act_thr()));
			break;
		}

		reset();
	}

	void	View::set_object(r_code::Code	*object){

		this->object=object;
		reset();
	}

	void	View::reset_ctrl_values(){

		sln_changes=0;
		acc_sln=0;
		act_changes=0;
		acc_act=0;
		vis_changes=0;
		acc_vis=0;
		res_changes=0;
		acc_res=0;

		periods_at_low_sln=0;
		periods_at_high_sln=0;
		periods_at_low_act=0;
		periods_at_high_act=0;
	}

	void	View::reset_init_sln(){

		initial_sln=get_sln();
	}

	void	View::reset_init_act(){

		if(object!=NULL)	
			initial_act=get_act();
		else
			initial_act=0;
	}

	float32	View::update_res(){

		float32	new_res=get_res();
		if(new_res==PLUS_INFINITY)
			return	new_res;
		if(res_changes>0	&&	acc_res!=0)
			new_res=get_res()+(float32)acc_res/(float32)res_changes;
		if(--new_res<0)	//	decremented by one on behalf of the group (at upr).
			new_res=0;
		code(VIEW_RES)=r_code::Atom::Float(new_res);
		acc_res=0;
		res_changes=0;
		return	get_res();
	}

	float32	View::update_sln(float32	low,float32	high){

		if(sln_changes>0	&&	acc_sln!=0){

			float32	new_sln=get_sln()+acc_sln/sln_changes;
			if(new_sln<0)
				new_sln=0;
			else	if(new_sln>1)
				new_sln=1;
			code(VIEW_SLN)=r_code::Atom::Float(new_sln);
		}
		acc_sln=0;
		sln_changes=0;

		float32	sln=get_sln();
		if(sln<low)
			++periods_at_low_sln;
		else{
			
			periods_at_low_sln=0;
			if(sln>high)
				++periods_at_high_sln;
			else
				periods_at_high_sln=0;
		}
		return	sln;
	}

	float32	View::update_act(float32	low,float32	high){

		if(act_changes>0	&&	acc_act!=0){

			float32	new_act=get_act()+acc_act/act_changes;
			if(new_act<0)
				new_act=0;
			else	if(new_act>1)
				new_act=1;
			code(VIEW_ACT)=r_code::Atom::Float(new_act);
		}
		acc_act=0;
		act_changes=0;

		float32	act=get_act();
		if(act<low)
			++periods_at_low_act;
		else{
			
			periods_at_low_act=0;
			if(act>high)
				++periods_at_high_act;
			else
				periods_at_high_act=0;
		}
		return	act;
	}

	float32	View::update_vis(){

		if(vis_changes>0	&&	acc_vis!=0){

			float32	new_vis=get_vis()+acc_vis/vis_changes;
			if(new_vis<0)
				new_vis=0;
			else	if(new_vis>1)
				new_vis=1;
			code(GRP_VIEW_VIS)=r_code::Atom::Float(new_vis);
		}
		acc_vis=0;
		vis_changes=0;
		return	get_vis();
	}

	void	View::delete_from_object(){

		object->acq_views();
		object->views.erase(this);
		if(object->views.size()==0)
			object->invalidate();
		object->rel_views();
	}

	void	View::delete_from_group(){

		Group	*g=get_host();
		g->enter();
		g->delete_view(this);
		g->leave();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	NotificationView::NotificationView(Code	*origin,Code	*destination,Code	*marker):View(){

		code(VIEW_OPCODE)=r_code::Atom::SSet(ViewOpcode,VIEW_ARITY);		//	Structured Set.
		code(VIEW_SYNC)=r_code::Atom::Float(View::SYNC_ONCE);				//	sync once.
		code(VIEW_IJT)=r_code::Atom::IPointer(VIEW_ARITY+1);				//	iptr to ijt.
		code(VIEW_SLN)=r_code::Atom::Float(1);								//	sln.
		code(VIEW_RES)=r_code::Atom::Float(_Mem::Get()->get_ntf_mk_res());	//	res.
		code(VIEW_HOST)=r_code::Atom::RPointer(0);							//	destination.
		code(VIEW_ORG)=r_code::Atom::RPointer(1);							//	origin.
		code(VIEW_ARITY+1)=r_code::Atom::Timestamp();						//	ijt will be set at injection time.
		code(VIEW_ARITY+2)=0;
		code(VIEW_ARITY+3)=0;
		references[0]=destination;
		references[1]=origin;

		reset_init_sln();

		object=marker;
	}
}