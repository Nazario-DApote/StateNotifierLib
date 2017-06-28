#pragma once

#include "Macho.hpp"
#include <iostream>
#include "StateNotifierLib.h"

using namespace std;

namespace SM {

	TOPSTATE(Top) {

		// state variables
		struct Box {
			Box() {}
			CStateNotifierLib* _stnotif;
		};

		STATE(Top)

		virtual void event() {}

		private:
			void entry();
			void exit();
			void init();

	}; // TOPSTATE

	SUBSTATE(OffState, Top) {
	   	STATE(OffState)

		void event();

	private:
	   	void entry();
	   	void exit();
	};

	SUBSTATE(OnState, Top) {
		STATE(OnState)

		void event();

	private:
		void entry();
		void exit();
	};

	////////////////////////////////////////////////////////////
	void Top::entry() {
		cout << "Top::entry" << endl;
	}
	void Top::exit() { cout << "Top::exit" << endl; }

	void Top::init() {
		setState<OffState>();
	}

	////////////////////////////////////////////////////////////
	void OffState::entry() {
		cout << "OffState::entry" << endl;
	}
	void OffState::exit() {
		cout << "OffState::exit" << endl;
	}
	void OffState::event() { setState<OnState>(); }

	////////////////////////////////////////////////////////////
	void OnState::entry() {
		cout << "OnState::entry" << endl;
	}
	void OnState::exit() {
		cout << "OnState::exit" << endl;
	}
	void OnState::event() { setState<OffState>(); }
}
