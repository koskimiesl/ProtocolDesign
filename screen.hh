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
	    void addList(std::vector<std::string> l);	
	    void showOption();
	    WINDOW * getIPWin();
	    void moveWinUp();
	    void moveWinDown();
	    void toggle();
		void switchtab();
		void status(const char []);
	private:
	    WINDOW * top;
		WINDOW * h1;
		WINDOW * h2;
	    WINDOW * mf;
	    WINDOW * ms;
        WINDOW * bottom;
		void showh12();
	    size_t win;
	    size_t cur;
		size_t tab;
	    std::map<std::string,bool> senlist;	
};

#endif


