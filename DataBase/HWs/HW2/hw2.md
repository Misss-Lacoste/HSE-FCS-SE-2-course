### 1.
```sql
CREATE TABLE Users (
    user_id SERIAL PRIMARY KEY,
    login VARCHAR(50) UNIQUE NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    email VARCHAR(100) UNIQUE NOT NULL,
    registration_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    is_confirmed BOOLEAN DEFAULT FALSE,
    recovery_token VARCHAR(255),
    recovery_token_expires TIMESTAMP
);

CREATE TABLE Friends (
    user_id INTEGER REFERENCES Users(user_id),
    friend_id INTEGER REFERENCES Users(user_id),
    status VARCHAR(20) CHECK (status IN ('pending', 'accepted', 'rejected')),
    request_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (user_id, friend_id)
);

CREATE TABLE Games (
    game_id SERIAL PRIMARY KEY,
    user_id INTEGER REFERENCES Users(user_id),
    name VARCHAR(100) NOT NULL,
    game_type VARCHAR(50) NOT NULL,
    min_players INTEGER NOT NULL,
    max_players INTEGER NOT NULL,
    duration INTEGER,
    description TEXT,
    image_url VARCHAR(255), 
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE Events (
    event_id SERIAL PRIMARY KEY,
    game_id INTEGER REFERENCES Games(game_id),
    organizer_id INTEGER REFERENCES Users(user_id),
    start_datetime TIMESTAMP NOT NULL,
    address VARCHAR(255),
    geocoordinates VARCHAR(100), 
    player_level VARCHAR(50),
    max_participants INTEGER NOT NULL,
    current_participants INTEGER DEFAULT 0,
    status VARCHAR(20) CHECK (status IN ('planned', 'cancelled', 'completed', 'failed')),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE EventParticipants (
    event_id INTEGER REFERENCES Events(event_id),
    user_id INTEGER REFERENCES Users(user_id),
    joined_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    status VARCHAR(20) CHECK (status IN ('joined', 'left', 'rejected')),
    PRIMARY KEY (event_id, user_id)
);

CREATE INDEX idx_events_datetime ON Events(start_datetime);
CREATE INDEX idx_events_geocoords ON Events(geocoordinates);
CREATE INDEX idx_events_status ON Events(status);
CREATE INDEX idx_games_type ON Games(game_type);
```
### 2.
```sql
SELECT 
    g.name AS game_name,
    e.event_id,
    e.start_datetime,
    e.address,
    e.geocoordinates,
    e.player_level,
    (e.max_participants - e.current_participants) AS free_spots
FROM 
    Events e
JOIN 
    Games g ON e.game_id = g.game_id
WHERE 
    e.start_datetime::DATE = '2020-15-05' 
    AND e.status = 'planned'
    AND e.current_participants < e.max_participants
ORDER BY 
    e.start_datetime;
```
### 3.
```sql
WITH event_pairs AS (
    SELECT DISTINCT
        ep1.user_id AS user1_id,
        ep2.user_id AS user2_id,
        g.game_type
    FROM 
        EventParticipants ep1
    JOIN 
        EventParticipants ep2 ON ep1.event_id = ep2.event_id
        AND ep1.user_id < ep2.user_id
    JOIN 
        Events e ON ep1.event_id = e.event_id
    JOIN 
        Games g ON e.game_id = g.game_id
    WHERE 
        ep1.status = 'joined' 
        AND ep2.status = 'joined'
        AND e.start_datetime >= CURRENT_DATE - INTERVAL '1 month'
),
game_counts AS (
    SELECT 
        user1_id, 
        user2_id, 
        game_type, 
        COUNT(*) AS games_played
    FROM 
        event_pairs
    GROUP BY 
        user1_id, user2_id, game_type
    HAVING 
        COUNT(*) >= 3
)
SELECT 
    gc.user1_id,
    gc.user2_id,
    gc.game_type,
    gc.games_played
FROM 
    game_counts gc
LEFT JOIN 
    Friends f ON (gc.user1_id = f.user_id AND gc.user2_id = f.friend_id)
    OR (gc.user1_id = f.friend_id AND gc.user2_id = f.user_id)
WHERE 
    f.user_id IS NULL
ORDER BY 
    gc.games_played DESC;
```
### 4.
```sql
SELECT 
    u.user_id,
    u.login,
    COALESCE(gc.game_count, 0) AS game_collection_size,
    COALESCE(ec.successful_events, 0) AS successful_events_organized,
    COALESCE(pc.failed_events, 0) AS failed_events_joined
FROM 
    Users u
LEFT JOIN (
    SELECT 
        user_id, 
        COUNT(*) AS game_count
    FROM 
        Games
    GROUP BY 
        user_id
) gc ON u.user_id = gc.user_id
LEFT JOIN (
    SELECT 
        organizer_id, 
        COUNT(*) AS successful_events
    FROM 
        Events
    WHERE 
        status = 'completed'
    GROUP BY 
        organizer_id
) ec ON u.user_id = ec.organizer_id
LEFT JOIN (
    SELECT 
        ep.user_id, 
        COUNT(*) AS failed_events
    FROM 
        EventParticipants ep
    JOIN 
        Events e ON ep.event_id = e.event_id
    WHERE 
        ep.status = 'joined' 
        AND e.status = 'failed'
    GROUP BY 
        ep.user_id
) pc ON u.user_id = pc.user_id
ORDER BY 
    u.user_id;
```
### 5. 
``` sql SELECT T1.A, SUM(T2.Y) 
FROM T1 JOIN T2 ON T1.A = T2.X 
GROUP BY T1.A;
```