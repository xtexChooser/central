use crate::ReqPrincipal;
use actix_web::{delete, get, post, put, web, HttpResponse};
use rauthy_api_types::groups::NewGroupRequest;
use rauthy_error::ErrorResponse;
use rauthy_models::entity::api_keys::{AccessGroup, AccessRights};
use rauthy_models::entity::groups::Group;

/// Returns all existing *groups*
///
/// **Permissions**
/// - rauthy_admin
#[utoipa::path(
    get,
    path = "/groups",
    tag = "groups",
    responses(
        (status = 200, description = "Ok", body = [Group]),
        (status = 401, description = "Unauthorized", body = ErrorResponse),
        (status = 403, description = "Forbidden", body = ErrorResponse),
    ),
)]
#[get("/groups")]
pub async fn get_groups(principal: ReqPrincipal) -> Result<HttpResponse, ErrorResponse> {
    principal.validate_api_key_or_admin_session(AccessGroup::Groups, AccessRights::Read)?;

    Group::find_all()
        .await
        .map(|rls| HttpResponse::Ok().json(rls))
}

/// Adds a new group to the database
///
/// **Permissions**
/// - rauthy_admin
#[utoipa::path(
    post,
    path = "/groups",
    tag = "groups",
    request_body = NewGroupRequest,
    responses(
        (status = 200, description = "Ok", body = Group),
        (status = 401, description = "Unauthorized", body = ErrorResponse),
        (status = 403, description = "Forbidden", body = ErrorResponse),
    ),
)]
#[post("/groups")]
pub async fn post_group(
    group_req: actix_web_validator::Json<NewGroupRequest>,
    principal: ReqPrincipal,
) -> Result<HttpResponse, ErrorResponse> {
    principal.validate_api_key_or_admin_session(AccessGroup::Groups, AccessRights::Create)?;

    Group::create(group_req.into_inner())
        .await
        .map(|r| HttpResponse::Ok().json(r))
}

/// Modifies a groups name
///
/// **Permissions**
/// - rauthy_admin
#[utoipa::path(
    put,
    path = "/groups/{id}",
    tag = "groups",
    request_body = NewGroupRequest,
    responses(
        (status = 200, description = "Ok", body = Group),
        (status = 401, description = "Unauthorized", body = ErrorResponse),
        (status = 403, description = "Forbidden", body = ErrorResponse),
    ),
)]
#[put("/groups/{id}")]
pub async fn put_group(
    id: web::Path<String>,
    group_req: actix_web_validator::Json<NewGroupRequest>,
    principal: ReqPrincipal,
) -> Result<HttpResponse, ErrorResponse> {
    principal.validate_api_key_or_admin_session(AccessGroup::Groups, AccessRights::Update)?;

    Group::update(id.into_inner(), group_req.group.to_owned())
        .await
        .map(|g| HttpResponse::Ok().json(g))
}

/// Deletes a group
///
/// It will be deleted from all currently assigned users too and this operation cannot be reverted.
///
/// **Permissions**
/// - rauthy_admin
#[utoipa::path(
    delete,
    path = "/groups/{id}",
    tag = "groups",
    responses(
        (status = 200, description = "Ok"),
        (status = 401, description = "Unauthorized", body = ErrorResponse),
        (status = 403, description = "Forbidden", body = ErrorResponse),
    ),
)]
#[delete("/groups/{id}")]
pub async fn delete_group(
    id: web::Path<String>,
    principal: ReqPrincipal,
) -> Result<HttpResponse, ErrorResponse> {
    principal.validate_api_key_or_admin_session(AccessGroup::Groups, AccessRights::Delete)?;

    Group::delete(id.into_inner())
        .await
        .map(|_| HttpResponse::Ok().finish())
}
