#include <fstream>
#include <lube.h>

using namespace std;
using namespace lube;

int main(int argc, char** argv)
{
    // Read a small file
    var f;
    var t;
    ifstream is(TEST_DIR "/test.txt", ifstream::in);
    if (is.fail())
        cout << "Open failed" << endl;
    while (f.getline(is))
    {
        cout << f << endl;
        t.push(f.copy());
    }
    cout << t << endl;

    // Read the same text file via a dynamic library
    filemodule txtmod("txt");
    file& txtf = txtmod.create();
    var txt = txtf.read(TEST_DIR "/test.txt");
    cout << "Loaded: " << txt << endl;

    // Read a .ini file via a dynamic library
    filemodule inimod("ini");
    file& inif = inimod.create();
    var ini = inif.read(TEST_DIR "/test.ini");
    cout << "Loaded: " << ini << endl;

    // Init from comma separated list
    var ai;
    ai = 5,4,3,2,1;
    cout << "ai is: " << ai << endl;

    // Plot it with gnuplot
    var gnu;
    gnu.push("plot sin(x), \"-\"");
    gnu.push(ai);
    filemodule gnumod("gnuplot");
    file& gnuf = gnumod.create();
    gnuf.write("test.eps", gnu);

    // gedcom
    filemodule gedmod("ged");
    file& gedf = gedmod.create();
    var ged = gedf.read(TEST_DIR "/test.ged");
    cout << "Loaded: " << ged << endl;

    // XML
    filemodule xmlmod("xml");
    file& xmlf = xmlmod.create();
    var xml = xmlf.read(TEST_DIR "/test.xml");
    cout << "Loaded: " << xml << endl;
    xmlf.write("test-out.xml", xml);

    // wav
    var sndAttr;
    sndAttr[lube::nil];
    filemodule wavmod("snd");
    file& wavf = wavmod.create(sndAttr);
    var wav = wavf.read(TEST_DIR "/test.wav");
    int dim = wav.dim();
    cout << "Loaded wav file:" << endl;
    cout << " rate:     " << sndAttr["rate"] << endl;
    cout << " channels: " << (dim > 1 ? wav.shape(0) : 1) << endl;
    cout << " frames:   " << wav.shape(dim-1) << endl;

    // Done
    return 0;
}
