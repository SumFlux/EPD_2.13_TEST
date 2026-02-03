# Contribution Guide

## Development Workflow

### 1. Environment Setup

Prerequisites:
- Python 3.9+
- MySQL 8.0+
- Redis 6.0+

```bash
# Create virtual environment
python -m venv venv
source venv/bin/activate  # Linux/Mac
# or
.\venv\Scripts\activate   # Windows

# Install dependencies
pip install -r requirements.txt

# Environment Configuration
cp .env.example .env
# Edit .env with your local database credentials
```

### 2. Database Management (Alembic)

We use Alembic for database migrations.

| Command | Description |
|---------|-------------|
| `alembic revision --autogenerate -m "msg"` | Generate new migration script from model changes |
| `alembic upgrade head` | Apply all pending migrations to database |
| `alembic downgrade -1` | Rollback last migration |

### 3. Running Locally

```bash
# Start development server with hot reload
uvicorn app.main:app --reload --host 0.0.0.0 --port 8000
```

- API Documentation: http://localhost:8000/docs
- Health Check: http://localhost:8000/health

### 4. Testing

```bash
# Run unit tests (if available)
pytest

# Run authentication verification script
python scripts/verify_auth_fix.py
```

### 5. Code Style

- Follow PEP 8
- Use Pydantic for data validation
- Async/Await for I/O bound operations
