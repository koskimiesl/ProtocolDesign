#include"screen.hh"

Screen::Screen(){
	initscr();
	cbreak();
	noecho();
	intrflush(stdscr, false);
    keypad(stdscr, true);
	// create window
	top = newwin(3,100,0,0);
	h1 = newwin(3,50,3,0);
	h2 = newwin(3,50,3,50);
	mf = newwin(25,50,6,0);
	ms = newwin(25,50,6,50);
	bottom = newwin(3,100,31,0);
	// create box around window
	box(top,ACS_VLINE,ACS_HLINE);
	box(mf,ACS_VLINE,ACS_HLINE);
	box(ms,ACS_VLINE,ACS_HLINE);
	
	// enable input for bottom window
	keypad(bottom, true);
	// write name of program
	wattron(top,A_BOLD);
	mvwprintw(top,1,40,"IoTPS Client");
	wattron(top,A_BOLD);	
	// reresh screens
	wrefresh(top);
	wrefresh(mf);
	wrefresh(ms);
	// initialize data members
	win = 0;
	cur = 0;
	tab = 0;
	showh12();
	status("Connecting...");
}

Screen::~Screen(){
	endwin();
}

WINDOW * Screen::getIPWin(){
	return bottom;
}

void Screen::addList(std::vector<std::string> l){
	std::vector<std::string>::iterator itr;
	for(itr = l.begin();itr != l.end();itr++)
		senlist.insert(std::pair<std::string,bool>(*itr,false));
	showOption();
}

void Screen::moveWinUp(){
	(cur > 0)?cur--:cur;
	(cur - win == 0 && win > 0)?win--:win;
	showOption();
}

void Screen::moveWinDown(){	
	(cur + 1 < senlist.size())?cur++:cur;
	(cur - win > 19)?win++:win;
	showOption();

}

void Screen::toggle(){
	size_t c;
	std::map<std::string,bool>::iterator itr;
	for(c = 0,itr = senlist.begin();itr != senlist.end();itr++,c++){
		if(c == cur){
			itr->second = itr->second?false:true;
			break;			
		}
	}
	showOption();
}

void Screen::showOption(){	
	std::map<std::string,bool>::iterator itr;	
	size_t c,i = 3;
	// erase and re-box
	werase(mf);
	box(mf,ACS_VLINE,ACS_HLINE);
	// Heading
	wattron(mf,A_BOLD);
	mvwprintw(mf,1,4,"Index");
	mvwprintw(mf,1,12,"Sensors");
	mvwprintw(mf,1,35,"Selection");
	wattroff(mf,A_BOLD);
	// write list
	for(c = 0,itr = senlist.begin();itr != senlist.end();itr++,c++){
		if(c >= win && c < win+20){
			if(c == cur)
				wattron(mf,A_STANDOUT);
			mvwprintw(mf,i++,4,"%-8u%-25s%c",c+1,itr->first.c_str(),itr->second?'#':' ');
			if(c == cur)			
				wattroff(mf,A_STANDOUT);
		}	
	}
	wrefresh(mf);
}

void Screen::switchtab(){
	tab=(tab==0)?1:0;
	showh12();
}

void Screen::showh12(){
	werase(h1);
	werase(h2);
	box(h1,ACS_VLINE,ACS_HLINE);
	box(h2,ACS_VLINE,ACS_HLINE);	
	if(tab == 0){
		wattron(h1,A_STANDOUT);
		mvwprintw(h1,1,20,"SUBSCRIBE");
		wattroff(h1,A_STANDOUT);
		mvwprintw(h2,1,20,"UNSUBSCRIBE");		
	}
	else {
		mvwprintw(h1,1,20,"SUBSCRIBE");
		wattron(h2,A_STANDOUT);
		mvwprintw(h2,1,20,"UNSUBSCRIBE");
		wattroff(h2,A_STANDOUT);
	}
	wrefresh(h1);
	wrefresh(h2);
}

void Screen::status(const char s[]){
	werase(bottom);
	box(bottom,ACS_VLINE,ACS_HLINE);	
	mvwaddstr(bottom,1,5,s);
	wrefresh(bottom);
}
