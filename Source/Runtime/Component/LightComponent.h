#pragma once

#include "LightComponentBase.h"

struct FLightConstants;

class ULightComponent : public ULightComponentBase
{
	DECLARE_CLASS(ULightComponent, ULightComponentBase)
	REFLECT_START(ULightComponent)
	REFLECT_END()
public:
	/*
	- 렌더링 스레드용 Light Scene Proxy 생성·관리
	- 렌더 씬에 광원을 등록하거나 제거
	- Transform과 속성 변경을 렌더링 스레드에 전달
	- 광원의 종류를 반환하는 공통 인터페이스
	- 광원의 위치를 표현하는 공통 인터페이스
	- 광원이 특정 Bounds에 영향을 주는지 판정
	- 그림자와 Shadow Map 관련 설정
	- Light Function Material
	- IES Profile
	- Specular Scale
	- Lighting Channels

	- Light Shaft
	- 광원 변경 시 렌더 상태 갱신
	*/

	// base는 일반 라이트와 스카이 라이트등 광원이 가지는 공통적인 특성을 정의한다.
	// 라이트 컴포넌트는 일반 라이트가 렌더되는 기능들을 추상적으로 관리
	// Register Fire Ball 을 통해 FireBallComponents에 등록하자
	// FireBallComponents에 등록된 애들이 UpdateFireBallLight에서 상수버퍼 구조체로 만들어지고
	// SetFireBallLight에서 상수버퍼로 등록됨
	
	// TODO: 
	// 우선 RegisterLight()을 만든다.
	// 다음으로 FireBall 로직들 이름을 Light로 변경하자
	// 
	ULightComponent() = default;
	virtual ~ULightComponent() override;
	virtual void InitializeComponent() override;
	virtual void UpdateConstants(FLightConstants& Constants) {};

protected:
	void SetIntensity(float NewIntensity) { Intensity = NewIntensity; }

private:



};