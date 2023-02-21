#pragma once

class InterfaceFramework;

struct TextBlock
{
    WCHAR                           m_pstrText[256];
    D2D1_RECT_F                     m_d2dLayoutRect;
    IDWriteTextFormat*              m_pdwFormat;
    ID2D1SolidColorBrush*           m_pd2dTextBrush;
};

class UILayer {
public:
    UILayer(InterfaceFramework& Iframe, UINT nTextBlocks);
    virtual ~UILayer();

    void UpdateTextOutputs(UINT nIndex, WCHAR* pstrUIText, D2D1_RECT_F* pd2dLayoutRect, IDWriteTextFormat* pdwFormat, ID2D1SolidColorBrush* pd2dTextBrush);
    virtual void Render(UINT nFrame);
    virtual void Build();
    void ReleaseResources();

    ID2D1SolidColorBrush* CreateBrush(D2D1::ColorF d2dColor);
    IDWriteTextFormat* CreateTextFormat(WCHAR* pszFontName, float fFontSize);

public:
    void InitializeDevice(ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, ID3D12Resource** ppd3dRenderTargets);
    void InitializeRenderTarget(UINT nFrame);

    float                           m_fWidth = 0.0f;
    float                           m_fHeight = 0.0f;

    ID3D11DeviceContext*            m_pd3d11DeviceContext = NULL;
    ID3D11On12Device*               m_pd3d11On12Device = NULL;
    IDWriteFactory*                 m_pd2dWriteFactory = NULL;
    ID2D1Factory3*                  m_pd2dFactory = NULL;
    ID2D1Device2*                   m_pd2dDevice = NULL;
    ID2D1DeviceContext2*            m_pd2dDeviceContext = NULL;

    UINT                            m_nRenderTargets = 0;
    ID3D11Resource**                m_ppd3d11WrappedRenderTargets = NULL;
    ID2D1Bitmap1**                  m_ppd2dRenderTargets = NULL;

    UINT                            m_nTextBlocks = 0;
    TextBlock*                      m_pTextBlocks = NULL;
};

//-------------------------------------------------------------------------------
/*	UILayerLobby_1 : UILayer	        									   */
//-------------------------------------------------------------------------------
class UILayerLobby_1 : public UILayer {
public:
    UILayerLobby_1(InterfaceFramework& Iframe, UINT nTextBlocks);
    virtual ~UILayerLobby_1();

    void Render(UINT nFrame) override;
    void Build() override;
};

//-------------------------------------------------------------------------------
/*	UILayerGame_1 : UILayer	        										   */
//-------------------------------------------------------------------------------
class UILayerGame_1 : public UILayer {
public:
    UILayerGame_1(InterfaceFramework& Iframe, UINT nTextBlocks);
    virtual ~UILayerGame_1();

    void Render(UINT nFrame) override;
    void Build() override;
};
