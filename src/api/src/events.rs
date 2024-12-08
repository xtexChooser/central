use crate::ReqPrincipal;
use actix_web::{get, post, web, HttpRequest, HttpResponse, Responder};
use actix_web_lab::sse;
use actix_web_validator::Json;
use chrono::Utc;
use rauthy_api_types::events::{EventsListenParams, EventsRequest};
use rauthy_common::constants::SSE_KEEP_ALIVE;
use rauthy_common::utils::real_ip_from_req;
use rauthy_error::{ErrorResponse, ErrorResponseType};
use rauthy_models::app_state::AppState;
use rauthy_models::entity::api_keys::{AccessGroup, AccessRights};
use rauthy_models::events::event::Event;
use rauthy_models::events::listener::EventRouterMsg;
use std::time::Duration;
use tokio::sync::mpsc;
use validator::Validate;

/// Get events
#[utoipa::path(
    post,
    path = "/events",
    tag = "events",
    responses(
        (status = 200, description = "Ok"),
        (status = 400, description = "BadRequest", body = ErrorResponse),
        (status = 401, description = "Unauthorized", body = ErrorResponse),
        (status = 403, description = "Forbidden", body = ErrorResponse),
    ),
)]
#[post("/events")]
pub async fn post_events(
    principal: ReqPrincipal,
    payload: Json<EventsRequest>,
) -> Result<HttpResponse, ErrorResponse> {
    principal.validate_api_key_or_admin_session(AccessGroup::Events, AccessRights::Read)?;

    payload.validate()?;
    let payload = payload.into_inner();

    let events = Event::find_all(
        payload.from,
        payload.until.unwrap_or_else(|| Utc::now().timestamp()),
        payload.level.into(),
        payload.typ.map(|t| t.into()),
    )
    .await?;

    Ok(HttpResponse::Ok().json(events))
}

/// Listen to the Events SSE stream
#[utoipa::path(
    get,
    path = "/events/stream",
    tag = "events",
    params(EventsListenParams),
    responses(
        (status = 200, description = "Ok"),
        (status = 400, description = "BadRequest", body = ErrorResponse),
        (status = 401, description = "Unauthorized", body = ErrorResponse),
        (status = 403, description = "Forbidden", body = ErrorResponse),
        (status = 404, description = "NotFound", body = ErrorResponse),
    ),
)]
#[get("/events/stream")]
pub async fn sse_events(
    data: web::Data<AppState>,
    principal: ReqPrincipal,
    params: web::Query<EventsListenParams>,
    req: HttpRequest,
) -> Result<impl Responder, ErrorResponse> {
    principal.validate_api_key_or_admin_session(AccessGroup::Events, AccessRights::Read)?;

    params.validate()?;

    let ip = real_ip_from_req(&req)?.to_string();
    let params = params.into_inner();
    let (tx, rx) = mpsc::channel(10);

    let level = params.level.map(|l| l.into()).unwrap_or_default();
    if let Err(err) = data
        .tx_events_router
        .send_async(EventRouterMsg::ClientReg {
            ip,
            tx,
            latest: params.latest,
            level,
        })
        .await
    {
        Err(ErrorResponse::new(
            ErrorResponseType::Internal,
            format!("Cannot register SSE client: {:?}", err),
        ))
    } else {
        Ok(sse::Sse::from_infallible_receiver(rx)
            .with_keep_alive(Duration::from_secs(*SSE_KEEP_ALIVE as u64))
            .with_retry_duration(Duration::from_secs(10)))
    }
}

/// Create a TEST Event
#[utoipa::path(
    post,
    path = "/events/test",
    tag = "events",
    responses(
        (status = 200, description = "Ok"),
        (status = 401, description = "Unauthorized", body = ErrorResponse),
        (status = 403, description = "Forbidden", body = ErrorResponse),
    ),
)]
#[post("/events/test")]
pub async fn post_event_test(
    data: web::Data<AppState>,
    principal: ReqPrincipal,
    req: HttpRequest,
) -> Result<HttpResponse, ErrorResponse> {
    principal.validate_api_key_or_admin_session(AccessGroup::Events, AccessRights::Create)?;

    Event::test(real_ip_from_req(&req)?)
        .send(&data.tx_events)
        .await?;

    Ok(HttpResponse::Ok().finish())
}
