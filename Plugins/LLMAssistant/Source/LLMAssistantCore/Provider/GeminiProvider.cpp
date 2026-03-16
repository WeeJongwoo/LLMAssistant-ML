#include "GeminiProvider.h"
#include "LLMAssistantSettings.h"

#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"

void FGeminiProvider::SendRequest(const FString& MessagesJson, FOnLLMResponseComplete OnComplete)
{
    const ULLMAssistantSettings* Settings = ULLMAssistantSettings::Get();

    // API 키 확인
    if (Settings->APIKey.IsEmpty())
    {
        OnComplete.ExecuteIfBound(false, TEXT(""), TEXT("API 키가 설정되지 않았습니다. Project Settings > LLM Assistant에서 입력하세요."));
        return;
    }

    // ── HTTP 요청 생성 ──
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(Settings->EndpointURL);
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

    // Gemini OpenAI 호환 엔드포인트는 Bearer 토큰 방식
    Request->SetHeader(TEXT("Authorization"),
        FString::Printf(TEXT("Bearer %s"), *Settings->APIKey));

    // ── JSON 바디 구성 ──
    // MessagesJson은 이미 [{"role":"user","content":"..."},..] 형태
    FString RequestBody = FString::Printf(
        TEXT("{\"model\":\"%s\",\"max_tokens\":%d,\"messages\":%s}"),
        *Settings->ModelName,
        Settings->MaxTokens,
        *MessagesJson
    );

    Request->SetContentAsString(RequestBody);

    // ── 응답 콜백 ──
    Request->OnProcessRequestComplete().BindLambda(
        [OnComplete](FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bConnectedSuccessfully)
        {
            // 네트워크 실패
            if (!bConnectedSuccessfully || !Resp.IsValid())
            {
                OnComplete.ExecuteIfBound(false, TEXT(""), TEXT("네트워크 연결에 실패했습니다."));
                return;
            }

            int32 ResponseCode = Resp->GetResponseCode();
            FString ResponseBody = Resp->GetContentAsString();

            // HTTP 에러
            if (ResponseCode != 200)
            {
                FString ErrorMsg = FString::Printf(
                    TEXT("API 오류 (HTTP %d): %s"), ResponseCode, *ResponseBody);
                OnComplete.ExecuteIfBound(false, TEXT(""), ErrorMsg);
                return;
            }

            // ── JSON 파싱 ──
            TSharedPtr<FJsonObject> JsonResponse;
            TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);

            if (!FJsonSerializer::Deserialize(Reader, JsonResponse) || !JsonResponse.IsValid())
            {
                OnComplete.ExecuteIfBound(false, TEXT(""), TEXT("응답 JSON 파싱에 실패했습니다."));
                return;
            }

            // OpenAI 호환 포맷: choices[0].message.content
            const TArray<TSharedPtr<FJsonValue>>* Choices;
            if (JsonResponse->TryGetArrayField(TEXT("choices"), Choices) && Choices->Num() > 0)
            {
                TSharedPtr<FJsonObject> FirstChoice = (*Choices)[0]->AsObject();
                TSharedPtr<FJsonObject> Message = FirstChoice->GetObjectField(TEXT("message"));
                FString Content = Message->GetStringField(TEXT("content"));

                OnComplete.ExecuteIfBound(true, Content, TEXT(""));
            }
            else
            {
                OnComplete.ExecuteIfBound(false, TEXT(""), TEXT("응답에서 choices를 찾을 수 없습니다."));
            }
        }
    );

    // 요청 전송
    Request->ProcessRequest();
}