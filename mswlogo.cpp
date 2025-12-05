// Full FLTK MSWLogo Clone - Standard C++ Only
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Menu_Bar.H>
#include <cmath>
#include <vector>
#include <string>
#include <map>
#include <sstream>
#include <fstream>
#include <iostream>
#include <cstdlib>

// ---------------------- Turtle Canvas ----------------------
struct Line { double x1,y1,x2,y2; };

class TurtleCanvas : public Fl_Widget {
public:
    double tx, ty, angle;
    bool penDown;
    double speed, penWidth;
    Fl_Color penColor;
    std::vector<Line> lines;

    TurtleCanvas(int X,int Y,int W,int H):Fl_Widget(X,Y,W,H) { reset(); }

    void reset() {
        tx=w()/2; ty=h()/2; angle=0; penDown=true;
        speed=1; penWidth=1; penColor=FL_BLACK;
        lines.clear(); redraw();
    }

    void forward(double d) {
        double nx=tx+d*cos(angle*M_PI/180), ny=ty-d*sin(angle*M_PI/180);
        if(penDown) lines.push_back({tx,ty,nx,ny});
        tx=nx; ty=ny; redraw(); Fl::wait(speed/1000.0);
    }

    void back(double d){ left(180); forward(d); right(180); }
    void right(double a){ angle-=a; redraw(); Fl::wait(speed/1000.0); }
    void left(double a){ angle+=a; redraw(); Fl::wait(speed/1000.0); }
    void penup(){ penDown=false; }
    void pendown(){ penDown=true; }
    void setpensize(double w){ penWidth=w; }
    void setpencolor(Fl_Color c){ penColor=c; }

    void draw() override {
        fl_push_clip(this->x(),this->y(),this->w(),this->h());
        fl_color(FL_WHITE); fl_rectf(this->x(),this->y(),this->w(),this->h());
        fl_color(penColor); fl_line_style(FL_SOLID, penWidth);
        for(auto &l:lines) fl_line(l.x1,l.y1,l.x2,l.y2);
        fl_color(FL_RED); fl_pie(tx-4,ty-4,8,8,0,360);
        fl_pop_clip();
    }

    void saveXML(const char* path){
        std::ofstream f(path);
        f<<"<Drawing>\n";
        for(auto &l:lines)
            f<<"  <Line x1='"<<l.x1<<"' y1='"<<l.y1<<"' x2='"<<l.x2<<"' y2='"<<l.y2<<"'/>\n";
        f<<"</Drawing>\n";
    }

    void loadXML(const char* path){
        std::ifstream f(path); if(!f.is_open()) return;
        lines.clear(); std::string line;
        while(std::getline(f,line)){
            double x1,y1,x2,y2;
            if(sscanf(line.c_str(),"  <Line x1='%lf' y1='%lf' x2='%lf' y2='%lf'/>\n",&x1,&y1,&x2,&y2)==4)
                lines.push_back({x1,y1,x2,y2});
        }
        redraw();
    }
};

// ---------------------- LOGO Interpreter ----------------------
class Interpreter {
public:
    TurtleCanvas* t;
    std::map<std::string,std::vector<std::string>> procs;
    std::map<std::string,double> vars;

    Interpreter(TurtleCanvas* canvas): t(canvas) {}

    void runLine(const std::string& line){
        std::istringstream iss(line);
        std::string cmd; iss>>cmd;

        if(cmd=="fd"){ double v; iss>>v; t->forward(v); }
        else if(cmd=="bk"){ double v; iss>>v; t->back(v); }
        else if(cmd=="rt"){ double v; iss>>v; t->right(v); }
        else if(cmd=="lt"){ double v; iss>>v; t->left(v); }
        else if(cmd=="pu"||cmd=="penup") t->penup();
        else if(cmd=="pd"||cmd=="pendown") t->pendown();
        else if(cmd=="setpensize"){ double w; iss>>w; t->setpensize(w); }
        else if(cmd=="setpencolor"){ int c; iss>>c; t->setpencolor((Fl_Color)c); }
        else if(cmd=="home") t->reset();
        else if(cmd=="clear"){ t->lines.clear(); t->redraw(); }
        else if(cmd=="ppt"){ std::cout<<"Turtle at ("<<t->tx<<","<<t->ty<<") angle="<<t->angle<<" penDown="<<t->penDown<<"\n"; }
        else if(cmd=="pe"){ t->penup(); t->lines.clear(); t->redraw(); }
        else if(cmd=="bye"){ exit(0); }
        else if(cmd=="repeat"){
            int n; iss>>n; std::string rest; std::getline(iss,rest);
            for(int i=0;i<n;i++) runScript(rest);
        }
        else if(cmd=="proc"){
            std::string name; iss>>name; procs[name]={};
        }
        else if(procs.count(cmd)){
            for(auto &c: procs[cmd]) runLine(c);
        }
        // Add variables, math, IF/WHILE later if desired
    }

    void runScript(const std::string& code){
        std::istringstream iss(code); std::string line;
        while(std::getline(iss,line)) runLine(line);
    }
};

// ---------------------- UI ----------------------
TurtleCanvas *canvas;
Fl_Text_Editor *editor;
Fl_Text_Buffer *buffer;
Interpreter *interp;

void run_cb(Fl_Widget*, void*){ interp->runScript(buffer->text()); }
void save_cb(Fl_Widget*, void*){ const char* f=fl_file_chooser("Save Drawing","*.xml","drawing.xml"); if(f) canvas->saveXML(f); }
void load_cb(Fl_Widget*, void*){ const char* f=fl_file_chooser("Load Drawing","*.xml","."); if(f) canvas->loadXML(f); }

int main(int argc,char **argv){
    Fl_Window win(1000,700,"MSWLogo FLTK - Full LOGO Commands");

    Fl_Menu_Bar *menu=new Fl_Menu_Bar(0,0,1000,25);
    menu->add("File/Save",0,save_cb,0);
    menu->add("File/Load",0,load_cb,0);

    canvas=new TurtleCanvas(400,25,600,675);

    buffer=new Fl_Text_Buffer();
    editor=new Fl_Text_Editor(0,25,400,650);
    editor->buffer(buffer);

    Fl_Button *runBtn=new Fl_Button(0,675,400,25,"Run");
    runBtn->callback(run_cb,0);

    win.end(); win.show();

    interp=new Interpreter(canvas);

    buffer->text(
        "fd 100\nrt 90\nfd 80\nlt 45\npu\nfd 50\npd\nsetpensize 3\nsetpencolor 4\nhome\nclear\nrepeat 4 [ fd 50 rt 90 ]\nppt\nbye\n"
    );

    return Fl::run();
}
