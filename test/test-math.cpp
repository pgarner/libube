#include "lube.h"
#include "lube/dft.h"

using namespace std;

// A test unary functor
class Unary : public lube::UnaryFunctor
{
public:
    Unary() { mDim = 1; };
protected:
    void vector(var iVar, var& oVar) const;
};

void Unary::vector(var iVar, var& oVar) const
{
    for (int i=0; i<iVar.size(); i++)
        oVar[i] = iVar[iVar.size()-1-i];
    cout << "Unary: I: " << iVar.shape() << " " << iVar << endl;
    cout << "Unary: O: " << oVar.shape() << " " << oVar << endl;
}


// A test N-ary functor
class Nary : public lube::NaryFunctor
{
public:
    Nary() { mDim = 1; };
protected:
    void vector(var iVar, var& oVar) const;
};

void Nary::vector(var iVar, var& oVar) const
{
    for (int i=0; i<iVar[0].size(); i++)
        oVar[i] = iVar[0][iVar.size()-1-i];
    cout << "Nary[0]: " << iVar[0].shape() << " " << iVar[0] << endl;
    cout << "Nary[1]: " << iVar[1].shape() << " " << iVar[1] << endl;
    cout << "Nary[2]: " << iVar[2].shape() << " " << iVar[2] << endl;
    cout << "Nary: O: " << oVar.shape() << " " << oVar << endl;
}

int main(int argc, char** argv)
{
    // Set the FP precision to be less than the difference between different
    // numerical libraries
    std::cout.precision(4);

    // BLAS
    var bt;
    bt = 1.0f, 1.2f, 0.8f, -2.0f;
    cout << "dim: " << bt.dim() << " shape: " << bt.shape() << endl;
    cout << "shape(0): " << bt.shape(0)
         << " shape(-1): " << bt.shape(-1) << endl;
    cout << bt << "  sums to " << lube::sum(bt)  << endl;
    cout << bt << " asums to " << lube::asum(bt) << endl;

    // Broadcasting scalars
    var t1 = lube::irange(16.0f).view({4, 4});
    t1(1,2) = 2.3f;
    t1 += 1;
    cout << "t1: " << t1 << endl;
    cout << "t1*0.5: " << t1 * 0.5 << endl;
    t1 *= 1.5;
    cout << "t1: " << t1 << endl;
    t1 += t1;
    cout << "t1: " << t1 << endl;
    t1 -= t1 - 1;
    cout << "t1: " << t1 << endl;

    // Sub-views
    var r12 = lube::irange(0.1, 12.1);
    cout << "r12: " << r12 << endl;
    var sv = r12.view({4}, 4);
    cout << "sv: " << sv << endl;
    cout << "sv(0): " << sv(0) << endl;
    cout << "sv[0]: " << sv[0] << endl;
    cout << "sv.size(): " << sv.size() << endl;
    sv += 1;
    cout << "r12: " << r12 << endl;
    var rv;
    rv = -1.0, -2.0, -3.0, -4.0;
    cout << "rv: " << rv << endl;
    sv.offset(8);
    sv = rv;
    cout << "r12: " << r12 << endl;

    // Broadcasting tensors
    var r6 = lube::irange(6.0).view({3,2});
    var r2;
    r2 = 1.0, 2.0;
    cout << "r2: " << r2 << endl;
    cout << "r6: " << r6 << endl;
    r6 += r2;
    cout << "r6: " << r6 << endl;
    r6 *= r2;
    cout << "r6: " << r6 << endl;
    cout << "r6*r6: " << r6 * r6 << endl;

    // View of a view
    var r3 = r6.view({2}, 0);
    var r4 = sv.view({2}, 0);
    cout << "r3: " << r3 << endl;
    cout << "r4: " << r4 << endl;

    // Functors
    cout << "Size: " << sizeof(lube::Pow) << endl;
    cout << "Pow: " << lube::pow(3.0, 2) << endl;
    cout << "Pow: " << lube::pow(r6, 2) << endl;
    cout << "Tan: " << lube::tan(r6) << endl;

    // Transpose
    cout << "oTranspose:\n" << lube::transpose(r6) << endl << r6 << endl;
    cout << "iTranspose:\n" << r6.transpose() << endl << r6 << endl;

    // Dot.  Start with single precision
    cout << "sdot: " << lube::dot(bt, bt) << endl;

    //Here, r6 is a 2x3 matrix
    var rd;
    rd = 2.0, 2.0, 2.0;
    var rd6 = lube::dot(r6, rd);
    cout << "Dot: " << rd6 << endl;

    // Matrix multiply
    var mm = var({1.0, 2.0, 3.0, 4.0}).view({2,2});
    cout << "MM: " << lube::dot(mm, r6) << endl;

    // DFT
    var td = lube::view({2, 10});
    var fd;
    for (int i=0; i<10; i++)
    {
        td(0, i) = sinf(i);
        td(1, i) = cosf(i);
    }
    cout << "Time: " << td << endl;
    lube::DFT dft(10);
    fd = dft(td);
    cout << "Freq: " << fd << endl;
    lube::IDFT idft(10);
    var ifd = idft(fd);
    cout << "IFreq: " << ifd << endl;

    // Check the complex operators
    cout << "Real: " << lube::real(fd) << endl;
    cout << "Imag: " << lube::imag(fd) << endl;
    cout << "Arg: " << lube::arg(fd) << endl;
    cout << "Abs: " << lube::abs(fd) << endl;
    cout << "Norm: " << lube::norm(fd) << endl;

    // Real output writing to complex storage
    var cd = var(12, lube::cfloat(0.0f,0.0f)).view({2,6});
    cout << "NormC: "<< lube::norm(fd, cd) << endl;

    var im = lube::iamax(fd);
    cout << "IAMax: " << im << endl;

    // Functor view broadcast
    Unary u;
    cout << u(r6) << endl;

    // N-ary functor
    var v1, v2, v3;
    v1 =
        0.0, 1.2, 1.4,
        1.0, 1.2, 1.4,
        2.0, 1.2, 1.4,
        3.0, 1.2, 1.4;
    v1 = v1.view({4,3});
    v2 =
        0.0, 2.2,
        1.0, 2.2,
        2.0, 2.2,
        3.0, 2.2;
    v2 = v2.view({4,2});
    v3 = 1.0, 2.2, 3.3, 4.4;
    Nary nary;
    var vN;
    vN.push(v1);
    vN.push(v2);
    vN.push(v3);
    cout << "vN: " << vN << endl;
    nary(vN);

    // Concatenate
    var vc = lube::concatenate({v1, v2});
    cout << "vc: " << v1 << v2 << vc << endl;
    var vc1({4.0,3.0,2.0,1.0});
    cout << "vc1: " << vc1 << lube::concatenate({vc,vc1.view({4,1})}) << endl;

    // Polynomial stuff
    var xpoly({1.0f, 0.0f, 0.0f, -1.0f});
    var xroot = lube::roots(xpoly);
    cout << "Roots of x^3-1=0: " << xroot << endl;
    xpoly = lube::poly(xroot);
    cout << "Poly is: " << xpoly << endl;

    // Sort
    var fsrt({1.0, -1.0, 2.1, -2.1, 3.0});
    cout << "Usrt is: " << fsrt << endl;
    cout << "Sort is: " << lube::sort(fsrt, fsrt) << endl;
    var csrt("Super duper strING");
    cout << "Usrt is: " << csrt << endl;
    cout << "Sort is: " << lube::sort(csrt, csrt) << endl;

    // Done
    return 0;
}
