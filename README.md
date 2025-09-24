## 블로그

관련 TIL1 : https://hnjog.github.io/unreal/c++/Task7_Base/<br>
관련 TIL2 : https://hnjog.github.io/unreal/c++/Task7_Challenge_6D/<br>
관련 TIL3 : https://hnjog.github.io/unreal/c++/Task7_Challenge2_Jump/<br>

## 구현 기능

필수 과제 관련 영상 : https://www.youtube.com/embed/XG3vRMyaywc <br>
도전 과제 1 (6D 관련) 관련 영상 : https://www.youtube.com/embed/B-StxcaJRkA <br>
도전 과제 2 (중력 관련) 관련 영상 : https://www.youtube.com/embed/uBF1pDQ7VpI <br>

필수 과제<br>
- Pawn 클래스 생성 및 CapsuleComponent를 Root로
- Skeltal Mesh, SpringArm, Camera 컴포넌트 생성 후 부착
- GameMode에서 DefaultPawnClass 지정
- Physics 설정 false
- Enhanced Input 매핑 & 바인딩 (Move & Look)
- 입력에 따른 '이동' 과 '회전' 구현
  (Controller 기반 함수 대신 AddActorLocalOffset, SetRelativeRotation 사용)

도전 과제 1<br>
- 6축 이동 및 회전 액션 구현<br>
  - 상/하, 좌/우, 전/후 이동<br>
  - Yaw , Pitch, Roll 회전 <br>
- Orientation 기반 이동<br>

도전 과제 2<br>
- 인공 중력 구현<br>
  - Tick + 임의의 중력 상수 사용<br>
  - LineTrace를 통하여 지면 충돌을 감지<br>

- 에어 컨트롤 및 궁중 이동 관성 구현<br>
  - JumpVelocity를 별도로 사용하여 기존 movement에 반영<br>

---

### 클래스 설명과 기타 파일

- 클래스

```less
TaskCharacter (APawn 상속)
- SetupPlayerInputComponent를 통해 
  Move, Look,Jump 에 대한 Input Bind & Function 구현
- 매핑에 따른 값을 통해 이동과 회전 구현
- AddActorLocalOffset, AddActorLocalRotation , SetRelativeRotation 사용
- LineTrace를 이용하여 바닥 도착 판정 추가

TaskPlayerController
- InputAction을 매개변수로 가지고,
  Player에게 IMC를 연결

ATaskHoverPawn
- 6D 구현용 Pawn 클래스
- Move 와 Look 은 방식과 유사하며
  입력으로 UpMove, DownMove 그리고 RotateZ 를 추가

AHoverController
- 6D 구현용 PlayerController 클래스

TaskGameMode
- DefaultPawn 및 PlayerController 세팅용 게임 모드

```

## 트러블 슈팅 1 - 회전이 제대로 먹히지 않는 문제

Look 을 구현하던 중 다음과 같은 문제가 발생하였다<br>

[![Image](https://github.com/user-attachments/assets/941d744d-3936-47ea-9dbe-917ad7458f7d)](https://github.com/user-attachments/assets/941d744d-3936-47ea-9dbe-917ad7458f7d){: .image-popup}<br>


카메라는 회전하지 않고,<br>
Mesh만 자기 멋대로 회전하는 상황이였다<br>

- 나는 Look에서 Rotator 값을 받고<br>
  Tick에서 Rotate에 해당하는 Rotate를 적용하는 방식을 사용중이었다<br>

```cpp
// Tick()
AddActorLocalRotation(FRotator(RotateR.Pitch * RotateSpeed * DeltaTime,RotateR.Yaw * RotateSpeed * DeltaTime,0.0));
```

그런데 생각해보니<br>
SpringArm에 `bUsePawnControlRotation = true`를 걸어두고<br>
'액터 전체'를 회전시키는 방식은<br>

내가 원하던 방식의 완벽한 반대였다...<br>

- CapsuleComponent가 Root 이므로 전체적인 회전을 걸어<br>
  Mesh도 회전됨<br>
  그런데 SpringArm은 Controller의 회전에만 영향을 받으므로<br>
  해당 회전은 적용되지 않음<br>

그렇기에 곰곰히 생각해본 결과<br>
다음과 같은 생각을 할 수 있었다<br>

- 먼저 우리는 지금 컨트롤러를 사용하지 않으니<br>
  해당 옵션은 끄기<br>

- Yaw 회전(좌우)은 '전체적'으로 적용하고<br>
  Pitch 회전(상하)은 'Spring Arm'에만 적용하기<br>

- 또한 Pitch 회전의 경우 카메라가 일정 범위에 도달할 경우<br>
  '제한'하기<br>
  (상하가 반대로 되버리는 것은 의도치 않았기에)<br>

- 마지막으로 이전 과제처럼 내가 바라보는 방향으로 이동하기를 원하였다<br>

그렇기에 그에 맞게 코드를 수정하니<br>

[![Image](https://github.com/user-attachments/assets/e4490923-3c0d-497c-ade1-e040a2f006be)](https://github.com/user-attachments/assets/e4490923-3c0d-497c-ade1-e040a2f006be){: .image-popup}<br>

원하는대로 구현되었다!<br>

Animation BP 쪽에서 GetVelocity로 속도를 받고 있었기에<br>
애니메이션이 정확히 나오지는 않지만<br>
그래도 꽤 만족스러운 결과이다<br>


## 트러블 슈팅 2 - 점프 이후, 내려오지 않는다...?

[![Image](https://github.com/user-attachments/assets/f8ac1e43-3084-4c51-a783-70aec99947ea)](https://github.com/user-attachments/assets/f8ac1e43-3084-4c51-a783-70aec99947ea){: .image-popup}<br>

점프 이후 '상승'만 하고<br>
캐릭터가 내려오지를 않는 상황<br>

```cpp
JumpVec.Z -= GravityScale * DeltaTime;
```

코드를 보았을때는 '중력 상수' 만큼 빼주고 있는데 왜이러지? 싶었다<br>

그런데 알고보니...<br>

```cpp
GravityScale = -980.0f;
```

값을 음수로... 잡아두고 있었다<br>

```cpp
JumpVec.Z += GravityScale * DeltaTime;
```

이렇게 수정하니 적당히 올라간 후<br>
땅으로 떨어지게 되었다<br>

## 트러블 슈팅 3 - 점프 순간의 관성이 유지되지 않는다?

[![Image](https://github.com/user-attachments/assets/f9a58402-cb11-4135-99b0-82dd48625b35)](https://github.com/user-attachments/assets/f9a58402-cb11-4135-99b0-82dd48625b35){: .image-popup}<br>

궁중에서 손을 '놓'으니<br>
그대로 '툭'하고 떨어진다!<br>

문제점이 무엇인가 확인하니<br>
사실상 JumpVec가 '방향'을 저장하는 역할 밖에 없었으며<br>
그것도 매우 미약하기에 Jump의 '관성'적인 역할은 거의 없었다<br>

```cpp
// TICK
if (JumpVec.Size() > 0)
{
	FVector TempJumpVec = JumpVec.GetSafeNormal() * MoveSpeed * DeltaTime;
	AddActorLocalOffset(TempJumpVec);

	if (UWorld* World = GetWorld())
	{
		FVector Start = GetActorLocation();
		FVector End = GetActorLocation() + GetActorUpVector() * -TraceHeight;

		FHitResult HitResult;

		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);

		bool bLand = World->LineTraceSingleByChannel(
			HitResult, Start, End, ECollisionChannel::ECC_Visibility, Params);

		if (bLand)
		{
			SetActorLocation(Start + GetActorUpVector() * CapsuleComponent->GetScaledCapsuleHalfHeight() / 2);
			bJump = false;
			JumpVec = FVector::Zero();
		}
	}

}

---

void ATaskCharacter::Jump(const FInputActionValue& value)
{
	if (value.Get<bool>() &&
		bJump == false)
	{
		bJump = true;
		JumpVec = FVector(MoveVec.X, MoveVec.Y, JumpSpeed);
	}
}
```

- 문제는 Normalize를 통해<br>
  JumpVec 자체가 '방향'처럼 쓰인다는 점...<br>

내가 원하는 건 점프 시점에서<br>
그 점프의 관성을 유지하는 것이기에<br>

Normalize를 제거하고<br>
대신 미리 Jump 시점에서 그 '관성'을 이용하기로 하였다<br>

```cpp
void ATaskCharacter::Jump(const FInputActionValue& value)
{
	if (value.Get<bool>() &&
		bJump == false)
	{
		bJump = true;
		JumpVec = FVector(MoveVec.X * MoveSpeed, MoveVec.Y * MoveSpeed, JumpSpeed);
	}
}
```

다만 이렇게 되니<br>
궁중 '이동' 중에서<br>
'손'을 놓는 경우<br>

다시 JumpVec가 저장한 방향대로 가기에<br>
역시 어색한 면이 존재하였다<br>

그렇기에<br>
JumpVec가 MoveVec에 영향을 받도록 수정하여<br>
관성과 '움직임' 모두를 챙기려고 하였다<br>

```cpp
if (JumpVec.Size() > 0)
{
	JumpVec.X += MoveVec.X * MoveSpeed * DeltaTime;
	JumpVec.Y += MoveVec.Y * MoveSpeed * DeltaTime;
	JumpVec.Z += GravityScale * DeltaTime;

	if (UWorld* World = GetWorld())
	{
		FVector Start = GetActorLocation();
		FVector End = GetActorLocation() + GetActorUpVector() * -TraceHeight;

		FHitResult HitResult;

		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);

		bool bLand = World->LineTraceSingleByChannel(
			HitResult, Start, End, ECollisionChannel::ECC_Visibility, Params);

		if (bLand)
		{
			SetActorLocation(Start + GetActorUpVector() * CapsuleComponent->GetScaledCapsuleHalfHeight() / 2);
			bJump = false;
			JumpVec = FVector::Zero();
		}
	}

	FVector TempJumpVec = JumpVec * DeltaTime;

	AddActorLocalOffset(TempJumpVec);
}
```

이제는 예시 영상처럼 잘 동작한다!<br>