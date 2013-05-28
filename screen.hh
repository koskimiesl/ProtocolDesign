#ifndef SCREEN_H
#define SCREEN_H

#include<ncurses.h>
#include<vector>
#include<map>
#include<cstdlib>
#include<cstring>
#include<iostream>
#include<string>
#include<sstream>

class Screen{
	public:
	    Screen();
	    ~Screen();
	    void addAList(std::vector<std::string> al); // available sensors
		void addSList(std::vector<std::string> sl);	// subscribed sensors
		std::vector<std::string> getSList();
		std::vector<std::string> getUList();
		std::vector<std::string> getPList();
	   	WINDOW * getIPWin();
	    void moveWinUp();
	    void moveWinDown();
	    void toggle();
		void switchtab();
		void status(const char []);
	private:
		// data
	    WINDOW * top,* hdr0,* hdr1,* opt[2],* bottom;
		size_t win[2];
	    size_t cur[2];
		size_t tab;
		std::map<std::string,bool> sensors[2];
		// functions
		void showTop();
		void showHeader();
		void showOption();
};

#endif


