#include"screen.hh"

Screen::Screen(){
	initscr();
	cbreak();
	noecho();
	intrflush(stdscr, false);
	// create window
	top = newwin(3,100,0,0);
	hdr0 = newwin(3,50,3,0);
	hdr1 = newwin(3,50,3,50);
	opt[0] = newwin(25,50,6,0);
	opt[1] = newwin(25,50,6,50);
	bottom = newwin(3,100,31,0);
	// enable input for bottom window
	keypad(bottom, true);
	// initialize data members
	win[0] = 0;
	win[1] = 0;
	cur[0] = 0;
	cur[1] = 0;
	// show screens
	showTop();
	tab = 1;
	showOption();
	tab = 0;
	showOption();
	showHeader();
	status("Press 'R' to retrive list.");
}

Screen::~Screen(){
	endwin();
}

void Screen::showTop(){
	box(top,ACS_VLINE,ACS_HLINE);
	// write name of program
	wattron(top,A_BOLD);
	mvwprintw(top,1,40,"IoTPS Client");
	wattron(top,A_BOLD);
	// refresh
	wrefresh(top);
}

void Screen::showHeader(){
	werase(hdr0);
	werase(hdr1);
	box(hdr0,ACS_VLINE,ACS_HLINE);
	box(hdr1,ACS_VLINE,ACS_HLINE);	
	if(tab == 0){
		wattron(hdr0,A_STANDOUT);
		mvwprintw(hdr0,1,20,"SUBSCRIBE");
		wattroff(hdr0,A_STANDOUT);
		mvwprintw(hdr1,1,20,"UNSUBSCRIBE");		
	}
	else {
		mvwprintw(hdr0,1,20,"SUBSCRIBE");
		wattron(hdr1,A_STANDOUT);
		mvwprintw(hdr1,1,20,"UNSUBSCRIBE");
		wattroff(hdr1,A_STANDOUT);
	}
	wrefresh(hdr0);
	wrefresh(hdr1);
}

void Screen::showOption(){	
	std::map<std::string,bool>::iterator itr;	
	size_t c,i = 3;
	// erase and re-box
	werase(opt[tab]);
	box(opt[tab],ACS_VLINE,ACS_HLINE);
	// Heading
	wattron(opt[tab],A_BOLD);
	mvwprintw(opt[tab],1,4,"Index");
	mvwprintw(opt[tab],1,12,"Sensors");
	mvwprintw(opt[tab],1,35,"Selection");
	wattroff(opt[tab],A_BOLD);
	// write list
	for(c = 0,itr = sensors[tab].begin();itr != sensors[tab].end();itr++,c++){
		if(c >= win[tab] && c < win[tab]+20){
			if(c == cur[tab])
				wattron(opt[tab],A_STANDOUT);
			mvwprintw(opt[tab],i++,4,"%-8u%-25s%c",c+1,itr->first.c_str(),itr->second?'X':'-');
			if(c == cur[tab])			
				wattroff(opt[tab],A_STANDOUT);
		}	
	}
	wrefresh(opt[tab]);
}

// add list of available sensors
void Screen::addAList(std::vector<std::string> al){
	std::vector<std::string>::iterator itr;
	for(itr = al.begin();itr != al.end();itr++)
		sensors[0].insert(std::pair<std::string,bool>(*itr,false));
	tab = 0;
	showOption();
}

// add list of subscribed sensors
void Screen::addSList(std::vector<std::string> sl){
	sensors[1].clear();
	std::vector<std::string>::iterator itr;
	for(itr = sl.begin();itr != sl.end();itr++)
		sensors[1].insert(std::pair<std::string,bool>(*itr,false));
	tab = 1;
	showOption();
}

std::vector<std::string> Screen::getSList(){
	std::vector<std::string> sl;
	std::map<std::string,bool>::iterator itr;
	for(itr = sensors[0].begin();itr != sensors[0].end();itr++){
		if(itr->second)
			sl.push_back(itr->first);
	}
	return sl;
}

std::vector<std::string> Screen::getUList(){
	std::vector<std::string> ul;
	std::map<std::string,bool>::iterator itr;
	for(itr = sensors[1].begin();itr != sensors[1].end();itr++){
		if(itr->second)
			ul.push_back(itr->first);
	}
	return ul;
}

std::vector<std::string> Screen::getPList(){
	std::vector<std::string> ul;
	std::map<std::string,bool>::iterator itr;
	for(itr = sensors[1].begin();itr != sensors[1].end();itr++){
		ul.push_back(itr->first);
	}
	return ul;
}

WINDOW * Screen::getIPWin(){
	return bottom;
}

void Screen::moveWinUp(){
	(cur[tab] > 0)?cur[tab]--:cur[tab];
	(cur[tab] - win[tab] == 0 && win[tab] > 0)?win[tab]--:win[tab];
	showOption();
}

void Screen::moveWinDown(){	
	(cur[tab] + 1 < sensors[tab].size())?cur[tab]++:cur[tab];
	(cur[tab] - win[tab] > 19)?win[tab]++:win[tab];
	showOption();
}

void Screen::toggle(){
	size_t c;
	std::map<std::string,bool>::iterator itr;
	for(c = 0,itr = sensors[tab].begin();itr != sensors[tab].end();itr++,c++){
		if(c == cur[tab]){
			itr->second = itr->second?false:true;
			break;			
		}
	}
	showOption();
}

void Screen::switchtab(){
	tab=(tab==0)?1:0;
	showHeader();
}

void Screen::status(const char s[]){
	werase(bottom);
	box(bottom,ACS_VLINE,ACS_HLINE);	
	mvwaddstr(bottom,1,5,s);
	wrefresh(bottom);
}
