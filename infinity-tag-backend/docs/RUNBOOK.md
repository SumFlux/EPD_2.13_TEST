# Runbook / Operations Guide

## Deployment Procedures

### Docker Deployment (Recommended)

The system is containerized using Docker Compose.

```bash
# 1. Build and Start Services
docker-compose up -d --build

# 2. View Logs
docker-compose logs -f backend

# 3. Stop Services
docker-compose down
```

### Environment Variables

| Variable | Description | Default |
|----------|-------------|---------|
| `APP_NAME` | Application Name | Infinity Tag Backend |
| `ENVIRONMENT` | deployment env (development/production) | production |
| `MYSQL_HOST` | Database Host | localhost |
| `MYSQL_PASSWORD` | Database Password | - |
| `REDIS_HOST` | Redis Host | localhost |
| `AI_API_KEY` | DMXAPI/OpenAI Key | - |
| `JWT_SECRET_KEY` | Secret for JWT Token generation | - |
| `FORCE_HTTPS` | Enforce HTTPS redirect | True |

See `.env.example` for full list.

## Monitoring & Troubleshooting

### Common Issues

#### 1. Database Connection Failed
- **Symptoms**: 500 Errors, "Access denied", "Can't connect to MySQL server".
- **Fix**:
  - Check `MYSQL_HOST` and `MYSQL_PORT` in `.env`.
  - If running in Docker, ensure host is `mysql`.
  - If running locally, ensure host is `localhost`.
  - Verify password matches local DB.

#### 2. AI Service Timeout
- **Symptoms**: 503 Service Unavailable on Almanac/Divination endpoints.
- **Fix**:
  - Check `AI_API_KEY` validity.
  - Verify network connectivity to `https://www.dmxapi.cn`.
  - Check logs: `docker-compose logs -f backend`.

#### 3. Port Conflicts
- **Symptoms**: `[Errno 10048] error while attempting to bind on address`.
- **Fix**:
  - Check if another instance is running: `lsof -i :8000` or `netstat -ano | findstr 8000`.
  - Kill the process or change port in `docker-compose.yml` / `uvicorn` command.

### Maintenance

#### Database Backup
```bash
docker-compose exec mysql mysqldump -u infinitytag -p infinity_tag > backup.sql
```

#### Log Rotation
Logs are currently output to stdout/stderr. Rely on Docker's logging driver for rotation.
