//	rgrp_overlay.h
//
//	Author: Eric Nivel
//
//	BSD license:
//	Copyright (c) 2010, Eric Nivel
//	All rights reserved.
//	Redistribution and use in source and binary forms, with or without
//	modification, are permitted provided that the following conditions are met:
//
//   - Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
//   - Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
//   - Neither the name of Eric Nivel nor the
//     names of their contributors may be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
//	THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
//	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//	DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
//	DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
//	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//	ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef	r_group_overlay_h
#define	r_group_overlay_h

#include	"overlay.h"
#include	"r_group.h"


namespace	r_exec{

	class	RGRPController;
	class	RGRPMasterOverlay;

	//	Overlays are used to index the value held by variable objects (bindings, stored per overlay) - bindings include nil values (i.e. yet unbound variables).
	//	Like program overlays hold a list of patterns that have stil to be matched, r-group overlays hold a list of the binders that still have to match.
	//	Binders are assumed to be the only ipgms held by r-groups.
	class	r_exec_dll	RGRPOverlay:
	public	Overlay{
	friend	class	RGRPMasterOverlay;
	private:
		uint64	birth_time;	//	same purpose as for pgm overlays.

		std::list<P<View> >				binders;	//	binders which remain to execute (a binder can execute only once per overlay).
		UNORDERED_MAP<Code	*,P<Code> >	bindings;	//	variable|value.
		std::vector<Code	*>			last_bound;	//	variables.
		uint16							unbound_var_count;
		bool							discard_bindings;

		RGRPOverlay(__Controller	*c,RGroup	*group,UNORDERED_MAP<Code	*,P<Code> >	*bindings=NULL);
		RGRPOverlay(RGRPOverlay	*original);
	public:
		~RGRPOverlay();

		void	reduce(r_exec::View	*input);
		void	bind(Code	*value,Code	*var);
	};

	//	Master r-group overlays.
	//	Acts as a sub-controller.
	//	See RGRPController comments.
	class	r_exec_dll	RGRPMasterOverlay:
	public	_Controller<Overlay>{
	friend	class	RGRPController;
	private:
		UNORDERED_MAP<Code	*,P<Code> >	bindings;	//	variable|value.

		RGRPMasterOverlay(RGRPController	*c,Code	*mdl,RGroup	*rgrp,UNORDERED_MAP<Code	*,P<Code> >	*bindings=NULL);
	public:
		~RGRPMasterOverlay();

		void	reduce(r_exec::View	*input);
		void	fire(UNORDERED_MAP<Code	*,P<Code> >	*bindings);
	};

	//	R-groups behave like programs: they take inputs from visible/(newly) salient views (depending on their sync mode).
	//	Overlays are built unpon the matching of at least one binder.
	//	Building an overlay means adding bindings (binding: a value for a given overlay) to variable objects.
	//	Binding is performed by the _subst function of the executive (see InputLessPGMOverlay::injectProductions()).
	//	The overlays held by controllers are r-group master overlays; these in turn hold r-group overlays.
	//	If the r-group has a parent, one master is built each time some parent overlay fires; otherwise there is only one master.
	//	If the parent overlay that built a master is killed (being too old), said master is removed from the conctollers, along with all its overlays;
	class	r_exec_dll	RGRPController:
	public	Controller{
	public:
		typedef	enum{
			HEAD=0,
			MIDDLE=1,
			TAIL=2
		}Position;
	private:
		RGroup	*rgrp;
		std::vector<P<View> >	pending_inputs;	//	stored before the controller becomes activated (i.e. before its parent fires).

		void	reduce(r_exec::View	*input);	//	convenience.
		void	injectProductions(UNORDERED_MAP<Code	*,P<Code> >	*bindings);
		Code	*bind_object(Code	*original,UNORDERED_MAP<Code	*,P<Code> >	*bindings)	const;
		Code	*get_mk_rdx(Code	*mdl,uint8	production_count,uint16	&extent_index)	const;

	public:
		RGRPController(_Mem	*mem,r_code::View	*view);	//	view is either a r-grp view (grp_view) or a mdl view (pgm_view).
		~RGRPController();

		Position	get_position()	const;

		void	activate(UNORDERED_MAP<Code	*,P<Code> >	*overlay_bindings,UNORDERED_MAP<Code	*,P<Code> >	*master_overlay_bindings);
		void	take_input(r_exec::View	*input,Controller	*origin=NULL);
		void	fire(UNORDERED_MAP<Code	*,P<Code> >	*overlay_bindings,UNORDERED_MAP<Code	*,P<Code> >	*master_overlay_bindings);
	};
}


#include	"rgrp_overlay.inline.cpp"


#endif