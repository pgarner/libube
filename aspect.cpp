#include <var>

using namespace std;

// See: http://jonisalonen.com/2012/converting-decimal-numbers-to-ratios/
var float2rat(var x)
{
    var tolerance = 1.e-2;
    var h1=1; var h2=0;
    var k1=0; var k2=1;
    var b = x;
    var ab;
    do
    {
        var a = b.floor();
        var aux = h1; h1 = a*h1+h2; h2 = aux;
        aux = k1; k1 = a*k1+k2; k2 = aux;
        b = var(1.0)/(b-a);
        ab = x-h1/k1;
    } while (x*tolerance < ab.abs());

    h1.push(k1);
    return h1;
}

int main(int argc, char** argv)
{
    var arg(argc, argv);

    var diag = 10.0;
    var res(2, 1024.0f, 600.0f);

    var aspect = res[0] / res[1];

    //var r = res[0]*res[0] + res[1]*res[1];
    var r = res.copy().pow(2).sum().sqrt();
    cout << "r: " << r << endl;
    var x = res[0] / r * diag;
    var y = res[1] / r * diag;
    var ratio = float2rat(aspect);
    cout << "Aspect: " << aspect << ", x y: " << x << " " << y << endl;
    cout << "Ratio: " << ratio[0] << ":" << ratio[1] << endl;

    var pitch = res.copy();
    pitch[0] /= x;
    pitch[1] /= y;
    cout << "Pitch: " << pitch << endl;

    return 0;
}
