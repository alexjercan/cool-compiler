-- TODO: add disconnect message/quit

class MessageHelper inherits Serde {
    deserialize(input: String): Tuple (* Message, String *) {
        let kind: String <- input.substr(0, 1),
            rest: String <- input.substr(1, input.length() - 1)
        in
            if kind = "0" then new PlayerPosition.deserialize(rest)
            else if kind = "1" then new CoinPosition.deserialize(rest)
            else if kind = "2" then new PlayerInput.deserialize(rest)
            else if kind = "3" then new PlayerConnected.deserialize(rest)
            else if kind = "4" then new PlayerAuthorize.deserialize(rest)
            else { abort(); new Tuple; } fi fi fi fi fi
    };
};

class PlayerPosition inherits Message {
    -- 1 byte for the kind of message: 0
    -- 8 bytes for the player id
    -- 8 bytes for the x position
    -- 8 bytes for the y position

    player_id: Int;
    pos_x: Int;
    pos_y: Int;

    init(id: Int, x: Int, y: Int): SELF_TYPE {
        {
            player_id <- id;
            pos_x <- x;
            pos_y <- y;
            self;
        }
    };

    player_id(): Int { player_id };
    pos_x(): Int { pos_x };
    pos_y(): Int { pos_y };

    serialize(): String {
        let id: String <- new MessageHelper.serialize_int(player_id),
            x: String <- new MessageHelper.serialize_int(pos_x),
            y: String <- new MessageHelper.serialize_int(pos_y)
        in "0".concat(id).concat(x).concat(y)
    };

    deserialize(input: String): Tuple (* Message, String *) {
        let id: Int <- new MessageHelper.deserialize_int(input.substr(0, 8)),
            x: Int <- new MessageHelper.deserialize_int(input.substr(8, 8)),
            y: Int <- new MessageHelper.deserialize_int(input.substr(16, 8)),
            msg: Message <- new PlayerPosition.init(id, x, y),
            rest: String <- input.substr(24, input.length() - 24)
        in new Tuple.init(msg, rest)
    };
};

class CoinPosition inherits Message {
    -- 1 byte for the kind of message: 1
    -- 8 bytes for the x position
    -- 8 bytes for the y position

    pos_x: Int;
    pos_y: Int;

    init(x: Int, y: Int): SELF_TYPE {
        {
            pos_x <- x;
            pos_y <- y;
            self;
        }
    };

    pos_x(): Int { pos_x };
    pos_y(): Int { pos_y };

    serialize(): String {
        let x: String <- new MessageHelper.serialize_int(pos_x),
            y: String <- new MessageHelper.serialize_int(pos_y)
        in "1".concat(x).concat(y)
    };

    deserialize(input: String): Tuple (* Message, String *) {
        let x: Int <- new MessageHelper.deserialize_int(input.substr(0, 8)),
            y: Int <- new MessageHelper.deserialize_int(input.substr(8, 8)),
            msg: Message <- new CoinPosition.init(x, y),
            rest: String <- input.substr(16, input.length() - 16)
        in new Tuple.init(msg, rest)
    };
};

class PlayerInput inherits Message {
    -- 1 byte for the kind of message: 2
    -- 8 bytes for the player id
    -- 1 bytes for the keyA
    -- 1 bytes for the keyD
    -- 1 bytes for the keyW
    -- 1 bytes for the keyS

    player_id: Int;
    keyA: Bool;
    keyD: Bool;
    keyW: Bool;
    keyS: Bool;

    init(id: Int, a: Bool, d: Bool, w: Bool, s: Bool): SELF_TYPE {
        {
            player_id <- id;
            keyA <- a;
            keyD <- d;
            keyW <- w;
            keyS <- s;
            self;
        }
    };

    keyA(): Bool { keyA };
    keyD(): Bool { keyD };
    keyW(): Bool { keyW };
    keyS(): Bool { keyS };

    serialize(): String {
        let id: String <- new MessageHelper.serialize_int(player_id),
            a: String <- new MessageHelper.serialize_byte(new Byte.from_int(keyA.to_int())),
            d: String <- new MessageHelper.serialize_byte(new Byte.from_int(keyD.to_int())),
            w: String <- new MessageHelper.serialize_byte(new Byte.from_int(keyW.to_int())),
            s: String <- new MessageHelper.serialize_byte(new Byte.from_int(keyS.to_int()))
        in "2".concat(id).concat(a).concat(d).concat(w).concat(s)
    };

    deserialize(input: String): Tuple (* Message, String *) {
        let id: Int <- new MessageHelper.deserialize_int(input.substr(0, 8)),
            a: Bool <- not new Byte.from_string(input.substr(8, 1)).to_int() = 0,
            d: Bool <- not new Byte.from_string(input.substr(9, 1)).to_int() = 0,
            w: Bool <- not new Byte.from_string(input.substr(10, 1)).to_int() = 0,
            s: Bool <- not new Byte.from_string(input.substr(11, 1)).to_int() = 0,
            msg: Message <- new PlayerInput.init(id, a, d, w, s),
            rest: String <- input.substr(12, input.length() - 12)
        in new Tuple.init(msg, rest)
    };
};

class PlayerConnected inherits Message {
    -- 1 byte for the kind of message: 3
    -- 8 bytes for the player id

    player_id: Int;

    init(id: Int): SELF_TYPE {
        {
            player_id <- id;
            self;
        }
    };

    player_id(): Int { player_id };

    serialize(): String {
        let id: String <- new MessageHelper.serialize_int(player_id)
        in "3".concat(id)
    };

    deserialize(input: String): Tuple (* Message, String *) {
        let id: Int <- new MessageHelper.deserialize_int(input.substr(0, 8)),
            msg: Message <- new PlayerConnected.init(id),
            rest: String <- input.substr(8, input.length() - 8)
        in new Tuple.init(msg, rest)
    };
};

class PlayerAuthorize inherits Message {
    -- 1 byte for the kind of message: 4
    -- 8 bytes for the player id

    player_id: Int;

    init(id: Int): SELF_TYPE {
        {
            player_id <- id;
            self;
        }
    };

    player_id(): Int { player_id };

    serialize(): String {
        let id: String <- new MessageHelper.serialize_int(player_id)
        in "4".concat(id)
    };

    deserialize(input: String): Tuple (* Message, String *) {
        let id: Int <- new MessageHelper.deserialize_int(input.substr(0, 8)),
            msg: Message <- new PlayerAuthorize.init(id),
            rest: String <- input.substr(8, input.length() - 8)
        in new Tuple.init(msg, rest)
    };
};
