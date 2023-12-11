
type cssColor = string;
type score = int;

let minScore = 150;
let diffScore = 5;

type player =
    | Human(cssColor,score)
    | AI(cssColor,score)
    | Ghost
; 

type field= 
    | Occupied(cssColor)
    | Empty
    ;

let isFieldEmpty = (field) => {
    switch(field){
        |Empty => true
        |_ => false
    };
};

let getFieldColor = field => switch(field){
    | Occupied(cssColor) => cssColor
    | _ => ""
}; 

type fieldMatrix = array(array(field));


type action = [
| `makeMove(Position.t)
| `reduceResultList(list(SquarePattern.t))
| `togglePlayer
| `aiNextStep(int)
| `showGameResult(player)
| `showMoveResult(int)
| `newGame
| `nothing
];


type state = {
    dim: Layout.dim,
    human: player,
    ai: player, 
    currPlayer: player,
    board: fieldMatrix,
    patterns: list(SquarePattern.t),  
    result: list(SquarePattern.t),
    action: action,
  };
type t = {
  mutable state: state,
  mutable renderer: GameRenderer.t,
  mutable firstMove: player
};


type loop('event,'state) = ('event, 'state) => state;
type interAction('event,'state) = ('event, 'state) => state;

let setup = (~dim:int, ~renderer: GameRenderer.t):t => {  
  {
    state: {
        dim,
        human:Human(Styles.humanColor,0),
        ai: AI(Styles.aiColor,0), 
        currPlayer: Ghost,   
        board: Array.make_matrix(dim, dim, Empty),
        patterns: SquarePattern.initSearchList(dim),
        result: [],
        action: `nothing,
    },
    renderer,
    firstMove: Ghost
  };
};

let newGame = ( dim ):state => {    
    {
        dim,
        human:Human(Styles.humanColor,0),
        ai: AI(Styles.aiColor,0),    
        currPlayer: Ghost,
        board: Array.make_matrix(dim, dim, Empty),
        patterns: SquarePattern.initSearchList(dim),
        result: [],       
        action: `nothing,
    };
};

let incrementPlayerHiScore = (player, score) => {
    switch(player){
        |Human(color,hiScore) => Human(color,hiScore+score)
        |AI(color,hiScore) => AI(color,hiScore+score)
        |_ => Ghost
    };
};

let getOtherPlayer = (state, player) => {    
    switch(player){
        | Human(_,_) => state.ai
        | AI(_,_)  => state.human
        | _ => Ghost
    };
};

let makeMove = (state, position, color):Position.checked => {
    let p0 = state.dim->Position.checkRange(position);
    switch(p0){
       |Valid(position) => {
           let (i,j) = position; 
           
           switch(state.board[i][j]){
               |Empty =>              
                    state.board[i][j] = Occupied(color);
                    Valid(position);               
               | _ => Invalid
           };
       }
       | _ => Invalid
    };
}; 

let undoMove = (state, position):Position.checked => {
    let p0 = state.dim->Position.checkRange(position);
    switch(p0){
       |Valid(position) => {
           let (i,j) = position; 
           
           switch(state.board[i][j]){
               |Occupied(_) =>              
                    state.board[i][j] = Empty;
                    Valid(position);               
               | _ => Invalid
           };
       }
       | _ => Invalid
    };
}; 

let getFieldAtPosition = (board,position) =>{ 
    let (i,j) = position;    
    board[i][j];    
};

let patternCoversSquare = (board: fieldMatrix, pattern:SquarePattern.t ) => {    
    let getField = getFieldAtPosition(board);
    switch(getField(pattern.p0),getField(pattern.p1),getField(pattern.p2),getField(pattern.p3),){
        | (Occupied(color0),Occupied(color1),Occupied(color2),Occupied(color3))  when color1==color0 && color2==color0 && color3==color0 =>
            pattern
        |_=> SquarePattern.null()
    };
};

let findAllSquares = (state, position) => { 

   SquarePattern.findAllSquaresWithSameCorner(position, state.patterns, state.dim)
    |> List.map(patternCoversSquare(state.board))
    |> List.filter(SquarePattern.notNull)
};

let getWinner = (player1,player2) => switch(player1,player2){
        | (Human(humanColor,humanScore),AI(aiColor,aiScore)) 
        | (AI(aiColor,aiScore),Human(humanColor,humanScore)) when max(aiScore,humanScore)>= minScore && abs(aiScore - humanScore) >= diffScore =>
                if(aiScore>humanScore){
                    AI(aiColor,aiScore);
                }else{
                    Human(humanColor,humanScore);
                }
        | _ => Ghost
    };
