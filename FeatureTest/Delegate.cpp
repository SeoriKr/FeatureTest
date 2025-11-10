#include <functional>
#include <unordered_map>
#include <memory>

using FDelegateHandle = size_t;

template<typename... Args>
class TDelegate
{
public:
    using HandlerType = std::function<void(Args...)>;

    TDelegate() : NextHandle(1) {}

    // 일반 함수나 람다 등록
    FDelegateHandle Add(const HandlerType& handler)
    {
        FDelegateHandle handle = NextHandle++;
        Handlers[handle] = handler;
        return handle;
    }

    // 클래스 멤버 함수 바인딩
    template<typename T>
    FDelegateHandle AddDynamic(T* Instance, void (T::* Func)(Args...))
    {
        if (Instance == nullptr)
        {
            return 0; // Invalid handle
        }

        auto handler = [Instance, Func](Args... args) {
            (Instance->*Func)(args...);
            };

        FDelegateHandle handle = NextHandle++;
        Handlers[handle] = handler;
        return handle;
    }

    // Const 멤버 함수 지원
    template<typename T>
    FDelegateHandle AddDynamic(T* Instance, void (T::* Func)(Args...) const)
    {
        if (Instance == nullptr)
        {
            return 0;
        }

        auto handler = [Instance, Func](Args... args) {
            (Instance->*Func)(args...);
            };

        FDelegateHandle handle = NextHandle++;
        Handlers[handle] = handler;
        return handle;
    }

    // 핸들로 특정 핸들러 제거
    bool Remove(FDelegateHandle handle)
    {
        auto it = Handlers.find(handle);
        if (it != Handlers.end())
        {
            Handlers.erase(it);
            return true;
        }
        return false;
    }

    // 모든 핸들러 호출
    void Broadcast(Args... args)
    {
        // 맵 복사본으로 순회 (실행 중 제거 안전성)
        auto handlersCopy = Handlers;
        for (const auto& pair : handlersCopy)
        {
            if (pair.second)
            {
                pair.second(args...);
            }
        }
    }

    // 모든 핸들러 제거
    void Clear()
    {
        Handlers.clear();
    }

    // 바인딩 여부 확인
    bool IsBound() const
    {
        return !Handlers.empty();
    }

    // 핸들러 개수
    size_t Num() const
    {
        return Handlers.size();
    }

private:
    std::unordered_map<FDelegateHandle, HandlerType> Handlers;
    FDelegateHandle NextHandle;
};

#define DECLARE_DELEGATE(Name, ...) using Name = TDelegate<__VA_ARGS__>
#define DECLARE_DELEGATE_NoParams(Name) using Name = TDelegate<>

// ========== 델리게이트 선언 ==========
DECLARE_DELEGATE(FOnHealthChanged, float, float);  // 이전값, 현재값
DECLARE_DELEGATE(FOnTakeDamage, float, class AActor*);  // 데미지, 공격자
DECLARE_DELEGATE_NoParams(FOnDeath);

// ========== Actor 클래스 ==========
class AActor
{
public:
    // 이벤트 델리게이트
    FOnHealthChanged OnHealthChanged;
    FOnTakeDamage OnTakeDamage;
    FOnDeath OnDeath;

    void TakeDamage(float Damage, AActor* Instigator)
    {
        float OldHealth = Health;
        Health -= Damage;

        // 데미지 이벤트 발생
        OnTakeDamage.Broadcast(Damage, Instigator);

        // 체력 변경 이벤트
        OnHealthChanged.Broadcast(OldHealth, Health);

        // 사망 체크
        if (Health <= 0.0f)
        {
            OnDeath.Broadcast();
        }
    }

protected:
    float Health = 100.0f;
};

// ========== 플레이어 클래스 ==========
class APlayer : public AActor
{
public:
    void BeginPlay()
    {
        // 자신의 이벤트에 핸들러 등록
        OnHealthChanged.AddDynamic(this, &APlayer::HandleHealthChanged);
        OnTakeDamage.AddDynamic(this, &APlayer::HandleTakeDamage);
        OnDeath.AddDynamic(this, &APlayer::HandleDeath);
    }

private:
    void HandleHealthChanged(float OldHealth, float NewHealth)
    {
        printf("Health changed: %.1f -> %.1f\n", OldHealth, NewHealth);
        // UI 업데이트 등
    }

    void HandleTakeDamage(float Damage, AActor* Instigator)
    {
        printf("Took %.1f damage!\n", Damage);
        // 피격 이펙트 재생 등
    }

    void HandleDeath()
    {
        printf("Player died!\n");
        // 사망 처리
    }
};

// ========== UI 클래스 ==========
class UHealthBar
{
public:
    void BindToActor(AActor* Actor)
    {
        if (Actor)
        {
            // 액터의 체력 변경 이벤트 구독
            HealthChangedHandle = Actor->OnHealthChanged.AddDynamic(
                this, &UHealthBar::OnActorHealthChanged);

            DeathHandle = Actor->OnDeath.AddDynamic(
                this, &UHealthBar::OnActorDeath);
        }
    }

    void Unbind(AActor* Actor)
    {
        if (Actor)
        {
            Actor->OnHealthChanged.Remove(HealthChangedHandle);
            Actor->OnDeath.Remove(DeathHandle);
        }
    }

private:
    void OnActorHealthChanged(float OldHealth, float NewHealth)
    {
        // 체력바 UI 업데이트
        float HealthPercent = NewHealth / 100.0f;
        printf("UI: Health bar updated to %.0f%%\n", HealthPercent * 100);
    }

    void OnActorDeath()
    {
        printf("UI: Showing death screen\n");
    }

    FDelegateHandle HealthChangedHandle;
    FDelegateHandle DeathHandle;
};

// ========== AI 컨트롤러 ==========
class AAIController
{
public:
    void PossessActor(AActor* Target)
    {
        TargetActor = Target;

        if (Target)
        {
            // 람다로도 등록 가능
            Target->OnTakeDamage.Add([this](float Damage, AActor* Instigator) {
                printf("AI: My pawn took damage, retaliating!\n");
                // 반격 로직
                });
        }
    }

private:
    AActor* TargetActor = nullptr;
};

// ========== 메인 사용 예제 ==========
int main()
{
    APlayer* Player = new APlayer();
    UHealthBar* HealthBar = new UHealthBar();
    AAIController* AIController = new AAIController();
    AActor* Enemy = new AActor();

    // 초기화
    Player->BeginPlay();
    HealthBar->BindToActor(Player);
    AIController->PossessActor(Player);

    printf("=== Test 1: Player takes damage ===\n");
    Player->TakeDamage(30.0f, Enemy);

    printf("\n=== Test 2: Player takes fatal damage ===\n");
    Player->TakeDamage(80.0f, Enemy);

    // 정리
    HealthBar->Unbind(Player);
    delete Player;
    delete HealthBar;
    delete AIController;
    delete Enemy;

    return 0;
}