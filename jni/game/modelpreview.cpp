#include "../main.h"
#include "RW/RenderWare.h"
#include "game.h"

struct Scene
{
	RpWorld *m_pRwWorld;
	RwCamera *m_pRwCamera;
};

stModelPreviewStuff modelPreviewStuff;

void InitModelPreviewStuff()
{
	RwRGBAReal color = { 1.0f, 1.0f, 1.0f, 1.0f };
	
	modelPreviewStuff.m_pRpLight = (( RpLight * (*)(int))(g_libGTASA + 0x001E3860 + 1))(2); // RpLightCreate(int)
	if(!modelPreviewStuff.m_pRpLight)
		return;
	
	// RpLightSetColor(RpLight *, RwRGBAReal const*)
	(( void (*)(RpLight *, RwRGBAReal *))(g_libGTASA + 0x001E333C + 1))(modelPreviewStuff.m_pRpLight, &color);
	
	modelPreviewStuff.m_pRwFrame = (( RwFrame * (*)())(g_libGTASA + 0x001AE9E0 + 1))(); // RwFrameCreate(void)
	
	if(!modelPreviewStuff.m_pRwFrame)
		return;
	
	RwV3d pos = { 0.0f, 0.0f, 50.0f };
	RwV3d rot = { 1.0f, 0.0f, 0.0f };
	
	// RwFrameTranslate(RwFrame *, RwV3d const*, RwOpCombineType)
	(( void (*)(RwFrame *, RwV3d *, int))(g_libGTASA + 0x001AED7C + 1))(modelPreviewStuff.m_pRwFrame, &pos, 0);  
	
	// RwFrameRotate(RwFrame *, RwV3d const*, float, RwOpCombineType)
	(( void (*)(RwFrame *, RwV3d *, float, int))(g_libGTASA + 0x001AEDC4 + 1))(modelPreviewStuff.m_pRwFrame, &rot, 90.0f, 1);

	modelPreviewStuff.m_pRwRaster = RwRasterCreate(256, 256, 0, 1);
	if(!modelPreviewStuff.m_pRwRaster)
		return;
	
	modelPreviewStuff.m_pRwCamera = (( RwCamera * (*)())(g_libGTASA + 0x001ADA1C + 1))(); // RwCameraCreate(void)
	if(!modelPreviewStuff.m_pRwCamera)
		return;
	
	modelPreviewStuff.m_pRwCamera->zBuffer = modelPreviewStuff.m_pRwRaster;
	
	// _rwObjectHasFrameSetFrame(void *, RwFrame *)
	(( void (*)(void *object, RwFrame *frame))(g_libGTASA + 0x1B2988 + 1))(&modelPreviewStuff.m_pRwCamera->object.object, modelPreviewStuff.m_pRwFrame);
	
 	// RwCameraSetFarClipPlane(RwCamera *, float)
	(( void (*)(RwCamera *, float))(g_libGTASA + 0x001AD710 + 1))(modelPreviewStuff.m_pRwCamera, 300.0f);  
	
	// RwCameraSetNearClipPlane(RwCamera *, float)
	(( void (*)(RwCamera *, float))(g_libGTASA + 0x001AD6F4 + 1))(modelPreviewStuff.m_pRwCamera, 0.01f);  

	RwV2d v2d = { 0.5f, 0.5f };
	
	// RwCameraSetViewWindow(RwCamera *, RwV2d const*)
	(( void (*)(RwCamera *, RwV2d *))(g_libGTASA + 0x001AD924 + 1))(modelPreviewStuff.m_pRwCamera, &v2d);
	
	// RwCameraSetProjection(RwCamera *, RwCameraProjection)
	(( void (*)(RwCamera *, int))(g_libGTASA + 0x001AD8DC + 1))(modelPreviewStuff.m_pRwCamera, 1);

	
	Scene * pScene = (Scene *)(g_libGTASA + 0x0095B060);  
	if(pScene)
	{
		(( void (*)(RpWorld *, RwCamera *))(g_libGTASA + 0x001EB118 + 1))(pScene->m_pRwWorld, modelPreviewStuff.m_pRwCamera);
	}
}

float  sub_774A4(unsigned int a1)
{
  float v1; // s0
  int v2; // r0
  int v3; // r0

  v1 = 0;
  if ( a1 <= 0x4E20 )
  {
    v2 = *(uint32_t *)(g_libGTASA + 4 * a1 + 0x0087BF48);// CModelInfo::ms_modelInfoPtrs
    if ( v2 )
    {
      v3 = *(uint32_t *)(v2 + 44);
      if ( v3 )
        v1 = *(float *)(v3 + 36);
    }
  }
  return v1;
}

RwTexture * CreateModelPreviewForVehicle(uint16_t vehicleId, CRGBA color, VECTOR vecRot, float fZoom, int iColor1, int iColor2)
{
	RwRaster *pRwRaster = RwRasterCreate(256, 256, 32, 1285);
	if(!pRwRaster)
		return 0;
	
	RwTexture *pTexture = RwTextureCreate(pRwRaster);
	if(!pTexture)
		return 0;
	
	uint16_t sVehicleId = vehicleId;
	
	if (sVehicleId == 569) sVehicleId = 537;
	else if (sVehicleId == 570) sVehicleId = 538;
	
	VECTOR pos = { 0.0f, 0.0f, 50.0f };
	CVehicle *pVehicle = new CVehicle(sVehicleId, pos, 0.0f, false);
	if(!pVehicle)
		return 0;
	
	pVehicle->m_pEntity->nEntityFlags.m_bUsesCollision = 0;
	pVehicle->m_pEntity->nEntityFlags.m_bCollisionProcessed = 0;
	
	float fOffsetY, fSomeCollisionValue = sub_774A4(sVehicleId);
 
	if(pVehicle->GetVehicleSubtype() == VEHICLE_SUBTYPE_BOAT)
		fOffsetY = fSomeCollisionValue * -2.5 + -5.5;
	else
		fOffsetY = fZoom * (-1.0 - (fSomeCollisionValue + fSomeCollisionValue));
	
	pVehicle->TeleportTo(VECTOR(0.0f, fOffsetY, 50.0f));
	
	if(iColor1 != -1 && iColor2 != -1)
		pVehicle->SetColor(iColor1, iColor2);
	
	MATRIX4X4 matrix;
	pVehicle->GetMatrix(&matrix);
	
	if(vecRot.X != 0.0f) RwMatrixRotate(&matrix, 0, vecRot.X);
	if(vecRot.Y != 0.0f) RwMatrixRotate(&matrix, 1, vecRot.Y);
	if(vecRot.Z != 0.0f) RwMatrixRotate(&matrix, 2, vecRot.Z);
	
	pVehicle->SetAndUpdateMatrix(matrix);
	
	modelPreviewStuff.m_pRwCamera->frameBuffer = pRwRaster;
	
	// CVisibilityPlugins::SetRenderWareCamera(RwCamera *)
	(( void (*)(RwCamera *))(g_libGTASA + 0x0055CFA4 + 1))(modelPreviewStuff.m_pRwCamera);
	
	//  RwCameraClear(RwCamera *, RwRGBA *, int)
	(( void (*)(RwCamera *, CRGBA *, int))(g_libGTASA + 0x001AD8A0 + 1))(modelPreviewStuff.m_pRwCamera, &color, 3);
	
	RwCameraBeginUpdate(modelPreviewStuff.m_pRwCamera);
	
	Scene * pScene = (Scene *)(g_libGTASA + 0x0095B060);  
	if(pScene)
	{
		// RpWorldAddLight(RpWorld *, RpLight *)
		(( void (*)(RpWorld *, RpLight *))(g_libGTASA + 0x001EB868 + 1))(pScene->m_pRwWorld, modelPreviewStuff.m_pRpLight);
	}
	
	RwRenderStateSet((RwRenderState)6,  (void*)1);
	RwRenderStateSet((RwRenderState)8,  (void*)1);
	RwRenderStateSet((RwRenderState)7,  (void*)0);
	RwRenderStateSet((RwRenderState)30, (void*)0);
	RwRenderStateSet((RwRenderState)20, (void*)0);
	RwRenderStateSet((RwRenderState)14, (void*)0);
	
	// DefinedState(void)
	(( void (*)())(g_libGTASA + 0x559008 + 1))();
	
	((void (*)(ENTITY_TYPE *))(*(void **)(pVehicle->m_pEntity->vtable + 0x8)))(pVehicle->m_pEntity);
	((void (*)(ENTITY_TYPE *))(*(void **)(pVehicle->m_pEntity->vtable + 72)))(pVehicle->m_pEntity);
	
	// CRenderer::RenderOneNonRoad(CRenderer *__hidden this, CEntity *)
	(( void (*)(ENTITY_TYPE *))(g_libGTASA + 0x003B1690 + 1))(pVehicle->m_pEntity);
	
	RwCameraEndUpdate(modelPreviewStuff.m_pRwCamera);
	
	if(pScene)
	{
		// RpWorldRemoveLight(RpWorld *, RpLight *)
		(( void (*)(RpWorld *, RpLight *))(g_libGTASA + 0x001EB8F0 + 1))(pScene->m_pRwWorld, modelPreviewStuff.m_pRpLight);
	}
	
	delete pVehicle;
	return pTexture;
}