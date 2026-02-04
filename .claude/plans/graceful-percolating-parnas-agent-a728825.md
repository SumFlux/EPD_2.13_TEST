# Implementation Plan: Image Customization Module Enhancements

## Goal
Add user-defined image customization features with quota limits, management APIs (web), and device synchronization.

## 1. Database Schema
- **New Model**: `CustomImage`
  - `id`: Integer, Primary Key
  - `user_id`: ForeignKey to `users.id`
  - `file_path`: String (Path to storage)
  - `display_order`: Integer (For sorting)
  - `created_at`: DateTime
- **Update Model**: `User`
  - Add `images` relationship (One-to-Many).

## 2. Service Layer (`ImageService`)
- **Quota Check**: Ensure user has < 5 images before upload.
- **File Management**: Save uploaded files to `assets/custom_images/{user_id}/`.
- **Reorder Logic**: Update `display_order` for a list of image IDs.
- **Sync Logic**: Fetch all images for a user, potentially packing them for the device (or just returning metadata if the device fetches one by one). *Requirement says "fetch entire queue at once" - likely a JSON list, or a binary pack if strictly "one request". JSON list + individual downloads is standard, but we'll provide the list first.*

## 3. API Endpoints (`/api/v1/images`)
- `POST /` (Upload): 
  - Input: Multipart file.
  - Checks quota.
  - Saves file, creates DB record.
- `GET /`: 
  - Returns list of images sorted by `display_order`.
  - Includes view count (if we add that field - requirement mentioned "View count"). *Update Schema to include `view_count`.*
- `DELETE /{id}`: 
  - Removes DB record and file.
- `PUT /reorder`:
  - Input: List of IDs in new order.
  - Updates `display_order`.
- `GET /sync` (Device):
  - Returns optimized list for device sync.

## 4. Frontend
- *Note: Frontend code is not present in this repository. API-only implementation.*

## Work Steps
1.  Define `CustomImage` model in `app/models/image.py`.
2.  Update `User` model in `app/models/user.py`.
3.  Create schema `ImageOut`, `ImageCreate` in `app/schemas/image.py`.
4.  Implement `ImageService` in `app/services/image_service.py`.
5.  Create API endpoints in `app/api/v1/endpoints/images.py`.
6.  Register router in `app/api/v1/api.py`.
