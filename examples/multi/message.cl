class MessageHelper {
    deserialize(input: String): Message {
        let kind: String <- input.substr(0, 1),
            rest: String <- input.substr(1, input.length() - 1)
        in
            if kind = "0" then new PlayerPosition.deserialize(rest)
            else if kind = "1" then new CoinPosition.deserialize(rest)
            else if kind = "2" then new PlayerInput.deserialize(rest)
            else if kind = "3" then new PlayerConnected.deserialize(rest)
            else if kind = "4" then new PlayerAuthorize.deserialize(rest)
            else { abort(); new Message; } fi fi fi fi fi
    };

    serialize_byte(byte: Byte): String { byte.to_string() };
    deserialize_byte(input: String): Byte { new Byte.from_string(input.substr(0, 1)) };

    serialize_int(int: Int): String {
        let b1: Byte <- new Byte.from_int(int.mod(256)),
            b2: Byte <- new Byte.from_int((int / 256).mod(256)),
            b3: Byte <- new Byte.from_int((int / 256 / 256).mod(256)),
            b4: Byte <- new Byte.from_int((int / 256 / 256 / 256).mod(256)),
            b5: Byte <- new Byte.from_int((int / 256 / 256 / 256 / 256).mod(256)),
            b6: Byte <- new Byte.from_int((int / 256 / 256 / 256 / 256 / 256).mod(256)),
            b7: Byte <- new Byte.from_int((int / 256 / 256 / 256 / 256 / 256 / 256).mod(256)),
            b8: Byte <- new Byte.from_int((int / 256 / 256 / 256 / 256 / 256 / 256 / 256).mod(256))
        in
            serialize_byte(b8).concat(serialize_byte(b7)).concat(serialize_byte(b6)).concat(serialize_byte(b5))
            .concat(serialize_byte(b4)).concat(serialize_byte(b3)).concat(serialize_byte(b2)).concat(serialize_byte(b1))
    };
    deserialize_int(input: String): Int {
        let b1: Byte <- deserialize_byte(input.substr(0, 1)),
            b2: Byte <- deserialize_byte(input.substr(1, 1)),
            b3: Byte <- deserialize_byte(input.substr(2, 1)),
            b4: Byte <- deserialize_byte(input.substr(3, 1)),
            b5: Byte <- deserialize_byte(input.substr(4, 1)),
            b6: Byte <- deserialize_byte(input.substr(5, 1)),
            b7: Byte <- deserialize_byte(input.substr(6, 1)),
            b8: Byte <- deserialize_byte(input.substr(7, 1))
        in
            b1.to_int() + b2.to_int() * 256 + b3.to_int() * 256 * 256 + b4.to_int() * 256 * 256 * 256
            + b5.to_int() * 256 * 256 * 256 * 256 + b6.to_int() * 256 * 256 * 256 * 256 * 256
            + b7.to_int() * 256 * 256 * 256 * 256 * 256 * 256 + b8.to_int() * 256 * 256 * 256 * 256 * 256 * 256 * 256
    };
};

class Message {
    serialize(): String { { abort(); ""; } };
    deserialize(input: String): Message { { abort(); new Message; } };
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

    deserialize(input: String): Message {
        let id: Int <- new MessageHelper.deserialize_int(input.substr(0, 8)),
            x: Int <- new MessageHelper.deserialize_int(input.substr(8, 8)),
            y: Int <- new MessageHelper.deserialize_int(input.substr(16, 8))
        in new PlayerPosition.init(id, x, y)
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

    deserialize(input: String): Message {
        let x: Int <- new MessageHelper.deserialize_int(input.substr(0, 8)),
            y: Int <- new MessageHelper.deserialize_int(input.substr(8, 8))
        in new CoinPosition.init(x, y)
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

    deserialize(input: String): Message {
        let id: Int <- new MessageHelper.deserialize_int(input.substr(0, 8)),
            a: Bool <- not new Byte.from_string(input.substr(8, 1)).to_int() = 0,
            d: Bool <- not new Byte.from_string(input.substr(9, 1)).to_int() = 0,
            w: Bool <- not new Byte.from_string(input.substr(10, 1)).to_int() = 0,
            s: Bool <- not new Byte.from_string(input.substr(11, 1)).to_int() = 0
        in new PlayerInput.init(id, a, d, w, s)
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

    deserialize(input: String): Message {
        let id: Int <- new MessageHelper.deserialize_int(input.substr(0, 8))
        in new PlayerConnected.init(id)
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

    deserialize(input: String): Message {
        let id: Int <- new MessageHelper.deserialize_int(input.substr(0, 8))
        in new PlayerAuthorize.init(id)
    };
};
