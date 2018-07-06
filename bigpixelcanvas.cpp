#include "bigpixelcanvas.h"
#include <wx/dcclient.h>

#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <fstream>

using namespace std;

BEGIN_EVENT_TABLE(BigPixelCanvas, wxPanel)
    EVT_PAINT    (BigPixelCanvas::OnPaint)
    EVT_LEFT_UP  (BigPixelCanvas::OnClick)
END_EVENT_TABLE()


inline wxColour operator-(const wxColour& c1, const wxColour& c2) {
    unsigned char red = c1.Red() - c2.Red();
    unsigned char green = c1.Green() - c2.Green();
    unsigned char blue = c1.Blue() - c2.Blue();
    return wxColour(red, green, blue);
}

inline wxColour operator*(const wxColour& c, float n) {
    unsigned char red = c.Red() * n;
    unsigned char green = c.Green() * n;
    unsigned char blue = c.Blue() * n;
    return wxColour(red, green, blue);
}

inline wxColour operator+(const wxColour& c1, const wxColour& c2) {
    unsigned char red = c1.Red() + c2.Red();
    unsigned char green = c1.Green() + c2.Green();
    unsigned char blue = c1.Blue() + c2.Blue();
    return wxColour(red, green, blue);
}

BigPixelCanvas::BigPixelCanvas(wxFrame *parent)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize),
      mPixelSize(1),
      mUsandoComandos(false),
      mBackgroundMode(wxSOLID),
      mColourForeground(*wxGREEN),
      mColourBackground(*wxWHITE),
      mPen(*wxBLACK_PEN)
{
    mOwnerPtr = parent;
    m_clip = false;
}

void BigPixelCanvas::DrawPixel(int x, int y, wxDC& dc)
{
    x *= mPixelSize;
    y *= mPixelSize;

    int halfPixelSize = mPixelSize / 2;
    int xStart = x - halfPixelSize;
    int xEnd = x + halfPixelSize;
    int yStart = y - halfPixelSize;
    int yEnd = y + halfPixelSize;
    for (int x = xStart; x <= xEnd; ++x)
        for (int y = yStart; y <= yEnd; ++y)
            dc.DrawPoint(x,y);
}

void BigPixelCanvas::DrawPixel(int x, int y, double z, wxDC& dc)
{
    if (mZBuffer.IsVisible(y, x, z)) {
        x *= mPixelSize;
        y *= mPixelSize;
        int halfPixelSize = mPixelSize / 2;
        int xStart = x - halfPixelSize;
        int xEnd = x + halfPixelSize;
        int yStart = y - halfPixelSize;
        int yEnd = y + halfPixelSize;
        for (int x = xStart; x <= xEnd; ++x)
            for (int y = yStart; y <= yEnd; ++y)
                dc.DrawPoint(x,y);
    }
}

void BigPixelCanvas::DrawLine(wxPoint p0, wxPoint p1)
{
    wxClientDC dc(this);
    PrepareDC(dc);
    p0 = ConvertDeviceToLogical(p0);
    p1 = ConvertDeviceToLogical(p1);
    DrawLine(p0, p1, dc);
}

void BigPixelCanvas::DrawLine(const wxPoint& p0, const wxPoint& p1, wxDC& dc){
    int p0x = p0.x;
    int p0y = p0.y;
    int p1x = p1.x;
    int p1y = p1.y;

    if (p0x > p1x){
        p0x = p1.x;
        p0y = p1.y;
        p1x = p0.x;
        p1y = p0.y;
    }

    int dy = p1y - p0y;
    int dx = p1x - p0x;

    int d, vE, vNe;

    int x = p0x;
    int y = p0y;

    int incremento = 1;
    if(dy < 0){
        incremento = -1;
        dy = -dy;
    }

    DrawPixel(x, y, dc);
    if(dy < dx){

        d = 2 * dy - dx;
        vE = 2 * dy;
        vNe = 2 * (dy - dx);

        while(x < p1x){
            if(d <= 0)
                d += vE;
            else {
                d += vNe;
                y += incremento;
            }
            ++x;
            DrawPixel(x, y, dc);
        }
    } else {

        d = 2 * dx - dy ;
        vE = 2 * dx;
        vNe = 2 * (dx - dy);

        int yCompar = y * incremento;
        p1y *= incremento;

        while(yCompar < p1y){
            if(d <= 0)
                d += vE;
            else {
                d += vNe;
                ++x;
            }
            y += incremento;
            ++yCompar;
            DrawPixel(x, y, dc);
        }
    }
}

void BigPixelCanvas::DrawCircle(wxPoint center, int radius){
    wxClientDC dc(this);
    PrepareDC(dc);
    center = ConvertDeviceToLogical(center);
    DrawCircle(center, radius/mPixelSize, dc);
}

void BigPixelCanvas::DrawCircle(const wxPoint& center, int radius, wxDC& dc){
    int d = 1 - radius;

    int y = radius;
    int x = 0;

    int limite = (int)(0.7071 * radius);           // limite de do 1º ao 2º octante

    DrawPixel(center.x, center.y, dc);             // centro
    DrawPixel(center.x, center.y + y, dc);
    DrawPixel(center.x + y, center.y, dc);
    DrawPixel(center.x, center.y - y, dc);
    DrawPixel(center.x - y, center.y, dc);

    while(y > limite){

        if(d < 0){
            d += 2 * x + 3;
        } else {
            d += 2 * (x - y) + 5;
            --y;
        }
        ++x;

        DrawPixel(center.x + x, center.y + y, dc);         //  x,  y
        DrawPixel(center.x + y, center.y + x, dc);         //  y,  x
        DrawPixel(center.x + y, center.y - x, dc);         //  y, -x
        DrawPixel(center.x + x, center.y - y, dc);         //  x, -y
        DrawPixel(center.x - x, center.y - y, dc);         // -x, -y
        DrawPixel(center.x - y, center.y - x, dc);         // -y, -x
        DrawPixel(center.x - y, center.y + x, dc);         // -y,  x
        DrawPixel(center.x - x, center.y + y, dc);         // -x,  y
    }

}

void BigPixelCanvas::DrawCircleAll(wxPoint center, int radius){
    wxClientDC dc(this);
    PrepareDC(dc);
    center = ConvertDeviceToLogical(center);
    DrawCircleAll(center, radius/mPixelSize, dc);
}

void BigPixelCanvas::DrawCircleAll(const wxPoint& center, int radius, wxDC& dc){
    int d = 1 - radius;
    int y = radius;
    int x = 0;

    int limite = (int)(0.707107 * radius);           // limite de do 1º ao 2º octante

    DrawPixel(center.x, center.y, dc);             // centro

    while(y >= limite){

        for(int i = y; i > 0; --i){
            DrawPixel(center.x + x, center.y + i, dc);         //  x,  y
            DrawPixel(center.x + i, center.y - x, dc);         //  y, -x
            DrawPixel(center.x - x, center.y - i, dc);         // -x, -y
            DrawPixel(center.x - i, center.y + x, dc);         // -y,  x
        }
        for(int i = y; i > limite; --i){
            DrawPixel(center.x + i, center.y + x, dc);         //  y,  x
            DrawPixel(center.x + x, center.y - i, dc);         //  x, -y
            DrawPixel(center.x - i, center.y - x, dc);         // -y, -x
            DrawPixel(center.x - x, center.y + i, dc);         // -x,  y
        }

        int pE = 2 * x + 3;
        int pSe = 2 * (x - y) + 5;

        if(d < 0){
            d += pE;
        } else {
            d += pSe;
            --y;
        }
        ++x;
    }
    
}


void BigPixelCanvas::DesenharTriangulo2D(const Triang2D& triangulo) {
    wxClientDC dc(this);
    PrepareDC(dc);
    DesenharTriangulo2D(triangulo, dc);
}

void BigPixelCanvas::DesenharTriangulo2D(const Triang2D& triangulo, wxDC& dc) {

    Ponto p1 = triangulo.P1();
    Ponto p2 = triangulo.P2();
    Ponto p3 = triangulo.P3();
    
    DrawLine(wxPoint(p1.mX, p1.mY), wxPoint(p2.mX, p2.mY), dc);     // P1 -> P2
    DrawLine(wxPoint(p2.mX, p2.mY), wxPoint(p3.mX, p3.mY), dc);     // P2 -> P3
    DrawLine(wxPoint(p3.mX, p3.mY), wxPoint(p1.mX, p1.mY), dc);     // P3 -> P1
}   



void BigPixelCanvas::DesenharTriangulo2DAll(const Triang2D& triangulo) {
    wxClientDC dc(this);
    PrepareDC(dc);
    DesenharTriangulo2DAll(triangulo, dc);
}

void BigPixelCanvas::DesenharTriangulo2DAll(const Triang2D& triangulo, wxDC& dc) {
    Ponto p1 = triangulo.P1();
    Ponto p2 = triangulo.P2();
    Ponto p3 = triangulo.P3();
    
    // variacao de Y de todos vetores (entre dois pontos)
    int vY12 = p2.mY - p1.mY;
    int vY23 = p3.mY - p2.mY;
    int vY13 = p3.mY - p1.mY;
    // mudulo da variação para testar qual é a maior
    // vY12 = (vY12 > 0)? vY12 : -vY12;
    // vY23 = (vY23 > 0)? vY23 : -vY23;
    // vY13 = (vY13 > 0)? vY13 : -vY13;

    int aLongaInicio_X, aLongaInicio_Y, aLongaFim_X, aLongaFim_Y, ponto_X, ponto_Y;
    if((abs(vY12) > abs(vY23)) and (abs(vY12) > abs(vY13))){
        aLongaInicio_X = p1.mX;
        aLongaInicio_Y = p1.mY;
        aLongaFim_X = p2.mX;
        aLongaFim_Y = p2.mY;
        ponto_X = p3.mX;
        ponto_Y = p3.mY;
    } else if((abs(vY13) > abs(vY23)) and (abs(vY13) > abs(vY12))){
        aLongaInicio_X = p1.mX;
        aLongaInicio_Y = p1.mY;
        aLongaFim_X = p3.mX;
        aLongaFim_Y = p3.mY;
        ponto_X = p2.mX;
        ponto_Y = p2.mY;
    } else {
        aLongaInicio_X = p2.mX;
        aLongaInicio_Y = p2.mY;
        aLongaFim_X = p3.mX;
        aLongaFim_Y = p3.mY;
        ponto_X = p1.mX;
        ponto_Y = p1.mY;
    }

    if(aLongaInicio_Y > aLongaFim_Y){
        int auxX = aLongaInicio_X;
        int auxY = aLongaInicio_Y;
        aLongaInicio_X = aLongaFim_X;
        aLongaInicio_Y = aLongaFim_Y;
        aLongaFim_X = auxX;
        aLongaFim_Y = auxY;
    }

    // cout << "Aresta longa Inicio_: " << aLongaInicio_X << " " << aLongaInicio_Y << endl;
    // cout << "Aresta longa Fim_: " << aLongaFim_X << " " << aLongaFim_Y << endl;
    // cout << "Aresta sobra   : " << ponto_X << " " << ponto_Y << endl;
    
    int yMin = aLongaInicio_Y;
    int yMax = ponto_Y;

    float xEsq = aLongaInicio_X;
    float xDir = aLongaInicio_X;

    // incremento da aresta longa, inc_a2 é o incremento que vai da aresta longa ate ponto
    // inc_a3 é o incremento do ponto ate o fim da aresta longa
    // pode chamar inc de derivação (onde teremos o coeficiente de variação)
    if(abs(aLongaFim_Y - aLongaInicio_Y) > 0 ){
        cout << "entrou" << endl;
        float inc_aLonga = ((float) (aLongaFim_X - aLongaInicio_X)) / (aLongaFim_Y - aLongaInicio_Y);
        float inc_a2 =     ((float) (ponto_X - aLongaInicio_X))     / (ponto_Y - aLongaInicio_Y);
        float inc_a3 =     ((float) (aLongaFim_X - ponto_X))        / (aLongaFim_Y - ponto_Y);

        float limiteTrianguloInf = inc_aLonga * (yMax - yMin) + aLongaInicio_X;

        // Ponto a ESQUERDA da aresta longa
        float incEsq = inc_a2;
        float incDir = inc_aLonga;
        bool pontoEstaDireita = limiteTrianguloInf < (float)(ponto_X);

        // para no caso de ponto estar a DIREITA da aresta longa
        if(pontoEstaDireita){
            incEsq = inc_aLonga;
            incDir = inc_a2;
        }

        int y = yMin;

        while(y < yMax){
            for(int x = xEsq; x < xDir; ++x){
                DrawPixel(x, y, dc);
            }
            xEsq += incEsq;
            xDir += incDir;

            ++y;
        }

        yMin = yMax;
        yMax = aLongaFim_Y;

        // Ponto a ESQUERDA da aresta longa
        incEsq = inc_a3;
        incDir = inc_aLonga;
        // para no caso de ponto estar a DIREITA da aresta longa
        if(pontoEstaDireita){
            incDir = inc_a3;
            incEsq = inc_aLonga;
        }

        y = yMin;
        while(y < yMax){
            for(int x = xEsq; x < xDir; ++x){
                DrawPixel(x, y, dc);
            }
            xEsq += incEsq;
            xDir += incDir;
            ++y;
        }
    }
}   

void BigPixelCanvas::DesenharTriangulo3D(const Triang3D& triangulo, wxDC& dc)
{
    Interv3D intervalo;
    while (triangulo.AtualizarIntervaloHorizontal(&intervalo))
        if (intervalo.Valido())
            DesenharIntervaloHorizontal(intervalo, dc);
}

void BigPixelCanvas::DesenharIntervaloHorizontal(const Interv2D& intervalo, wxDC& dc)
{
    int x = intervalo.mXMin;
    while (x < intervalo.mXMax) {
        DrawPixel(x, intervalo.mY, dc);
        ++x;
    }
}

void BigPixelCanvas::DesenharIntervaloHorizontal(const Interv3D& intervalo, wxDC& dc)
{
    // Colocar aqui o código para desenhar um intervalo horizontal 3D. Necessário
    // para a implementação do z-buffer.
    // Desenhar um intervalo 3D é como desenhar um intervalo 2D, usando z-buffer.
    #warning BigPixelCanvas::DesenharIntervaloHorizontal não foi implementado (necessário para a rasterização do z-buffer).
}

void BigPixelCanvas::OnPaint(wxPaintEvent& event)
{
    wxPaintDC pdc(this);
    wxDC &dc = pdc;

    PrepareDC(dc);

    mOwnerPtr->PrepareDC(dc);
    dc.SetBackgroundMode( mBackgroundMode );
    if ( mBackgroundBrush.Ok() )
        dc.SetBackground( mBackgroundBrush );
    if ( mColourForeground.Ok() )
        dc.SetTextForeground( mColourForeground );
    if ( mColourBackground.Ok() )
        dc.SetTextBackground( mColourBackground );

    dc.Clear();
    if (mUsandoComandos)
        InterpretarComandos();
}

void BigPixelCanvas::InterpretarComandos()
{
    ifstream arquivo("comandos.txt");
    wxClientDC dc(this);
    PrepareDC(dc);
    string comando;
    while (arquivo >> comando)
    {
        if (comando == "linha")
        {
            int p0x, p0y, p1x, p1y;
            arquivo >> p0x >> p0y >> p1x >> p1y;
            DrawLine(wxPoint(p0x, p0y), wxPoint(p1x, p1y), dc);
        }
        else if (comando == "cor")
        {
            int r, g, b;
            arquivo >> r >> g >> b;
            mPen.SetColour(r, g, b);
            dc.SetPen(mPen);
        }
        else if (comando == "triangulo3d")
        {
            int x, y, z;
            arquivo >> x >> y >> z;
            P3D p1(x,y,z);
            arquivo >> x >> y >> z;
            P3D p2(x,y,z);
            arquivo >> x >> y >> z;
            P3D p3(x,y,z);
            Triang3D tri(p1, p2, p3);
            DesenharTriangulo3D(tri, dc);
        }
    }
}

void BigPixelCanvas::OnClick(wxMouseEvent &event)
{
    wxPostEvent(mOwnerPtr, event);
}

void BigPixelCanvas::PrepareDC(wxDC& dc)
{
    int height, width;
    GetClientSize(&width, &height);
    dc.SetLogicalOrigin(-width/2, height/2);
    dc.SetAxisOrientation(true, true);
    dc.SetMapMode(wxMM_TEXT);
    dc.SetPen(mPen);
    mZBuffer.AlterarCapacidade(static_cast<unsigned int>(height/mPixelSize),
                               static_cast<unsigned int>(width/mPixelSize));
}

wxPoint BigPixelCanvas::ConvertDeviceToLogical(const wxPoint& p)
{
    wxClientDC dc(this);
    PrepareDC(dc);
    wxPoint result;
    result.x = dc.DeviceToLogicalX(p.x) / mPixelSize;
    result.y = dc.DeviceToLogicalY(p.y) / mPixelSize;
    return result;
}

