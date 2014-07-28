#include <fstream>
#include <lv.h>

using namespace std;

int main(int argc, char** argv)
{
    // Read a small file
    var f;
    var t;
    ifstream is("test.txt", ifstream::in);
    if (is.fail())
        cout << "Open failed" << endl;
    while (f.getline(is))
    {
        cout << f << endl;
        t.push(f.copy());
    }
    cout << t << endl;

    // Read a text file via a dynamic library
    vfile txtf("txt");
    var txt = txtf.read("test.txt");
    cout << "Loaded: " << txt << endl;

    // Read a .ini file via a dynamic library
    vfile inif("ini");
    var ini = inif.read("test.ini");
    cout << "Loaded: " << ini << endl;

    // Init from comma separated list
    var ai;
    ai = 5,4,3,2,1;
    cout << "ai is: " << ai << endl;

    // Plot it with gnuplot
    var gnu;
    gnu.push("plot sin(x), \"-\"");
    gnu.push(ai);
    vfile gnuf("gnuplot");
    gnuf.write("test.eps", gnu);

    // gedcom
    vfile gedf("ged");
    var ged = gedf.read("test.ged");
    cout << "Loaded: " << ged << endl;

    // XML
    vfile xmlf("xml");
    var xml = xmlf.read("test.xml");
    cout << "Loaded: " << xml << endl;

    // wav
    vfile wavf("snd");
    var wav = wavf.read("test.wav");
    cout << "Loaded wav file:" << endl;
    cout << " rate:     " << wav["rate"] << endl;
    cout << " frames:   " << wav["data"].shape(0) << endl;
    cout << " channels: " << wav["data"].shape(1) << endl;

    // Done
    return 0;
}
