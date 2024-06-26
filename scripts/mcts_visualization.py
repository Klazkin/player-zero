from ete3 import Tree, TreeStyle, TextFace, AttrFace, faces, TreeNode, RectFace, NodeStyle
from ete3.treeview.faces import add_face_to_node
from enum import auto, Enum

with open("graph.data", "r") as f:
    graph = f.read()


class Action(Enum):
    INVALID_ACTION = -1
    TREAD = auto()
    COILBLADE = auto()
    SWIFTARROW = auto()
    ALIGNMENT_LANCE = auto()
    WRATHSPARK = auto()
    GROUNDRAISE = auto()
    BLOODDRAWING = auto()
    WISPSPARKS = auto()
    BONEDUST = auto()
    RESPIRIT = auto()
    SNOWMOTES = auto()
    ETERNALSHACLES = auto()
    ALTAR = auto()
    END_TURN = auto()
    COMBINE_ACTIONS = auto()
    NETHERSWAP = auto()
    LOS_ACTION = auto()
    DETONATION = auto()
    DEBUG_KILL = auto()
    BONESPARKS = auto()
    SUNDIVE = auto()
    METEORSHATTER = auto()
    ARMORCORE = auto()
    IMMOLATION = auto()
    ICEPOLE = auto()
    OBLIVION = auto()
    HOARFROST = auto()
    RAPID_GROWTH = auto()
    SUBLIMESTRUCTURE = auto()
    BLESSING = auto()


action_name_dict = {
    Action.END_TURN: "End Turn",
    Action.COMBINE_ACTIONS: "Action Combination",
    Action.INVALID_ACTION: "Invalid action",
    # Action.IDLE: "Idle",
    Action.WRATHSPARK: "Simple Damage",
    Action.GROUNDRAISE: "Ground",
    Action.BLOODDRAWING: "Blood",
    Action.TREAD: "Move",
    Action.COILBLADE: "Sword",
    Action.ETERNALSHACLES: "Shacles",
    Action.ALTAR: "Altar",
    Action.NETHERSWAP: "Netherswap",
    Action.LOS_ACTION: "Action LOS",
    Action.BONEDUST: "Bonedust",
    Action.WISPSPARKS: "Wispsparks",
    Action.BONESPARKS: "Bonesparks",
    Action.DEBUG_KILL: "Insta Kill",
    Action.DETONATION: "Explode",
    Action.RESPIRIT: "Respirit",
    Action.SNOWMOTES: "Snowmotes",
    Action.SUNDIVE: "Sundive",
    Action.METEORSHATTER: "Meteor",
    Action.ARMORCORE: "Armorcore",
    Action.IMMOLATION: "Immolation",
    Action.ICEPOLE: "Icepole",
    Action.OBLIVION: "Oblivion",
    Action.HOARFROST: "Hoarfrost",
    Action.RAPID_GROWTH: "Rapid Growth",
    Action.SWIFTARROW: "Bow",
    Action.ALIGNMENT_LANCE: "Lance",
}

HORIZONTAL_STYLE = False
TOP_ONLY = True


def layout(node: TreeNode, max_depth=5):
    node_depth = len(node.get_ancestors())
    node.img_style["fgcolor"] = "0"
    node.img_style["shape"] = "sphere"

    if node_depth > max_depth:
        for child in node.get_children():
            child.detach()

        return

    action_id, target_x, target_y, visits, score, policy, faction = str.split(node.name, "/")
    node.add_features(faction=faction)

    if TOP_ONLY and not node.is_root() and node.get_ancestors()[0].faction != faction:
        for child in node.get_children():
            child.detach()

        return

    action = Action(int(action_id))

    def get_exploit(n_score: str, n_visits: str) -> float:
        return round(float(n_score), 5) if float(n_visits) > 0 else 0 # /float(n_visits)

    exploit = get_exploit(score, visits)

    def get_action_name(a: str):
        return action_name_dict.get(Action(int(a)), "Unknown")

    action_name = "Root" if node.is_root() else action_name_dict.get(action, "Unknown")
    if action == Action.COMBINE_ACTIONS:
        action_target = f"({get_action_name(target_x)},{get_action_name(target_y)})"
    else:
        action_target = f"({target_x},{target_y})"

    join_symbol = ' ' if (node.is_leaf() or ((max_depth - node_depth) == 0)) and not HORIZONTAL_STYLE else "\n"

    faction_sign = "P" if faction == "0" else "M"

    action_info = join_symbol.join([
        f"{faction_sign}: {action_name}",
        action_target,
        f"v={visits} e={exploit} policy={policy}"
    ])

    max_policy_path = True
    max_exploit_path = True
    max_visits_path = True

    for sis in node.get_sisters():
        try:
            _, _, _, sis_visits, sis_score, sis_policy, _ = str.split(sis.name, "/")
        except Exception as e:
            print(sis.name)
            raise

        if float(sis_policy) > float(policy):
            max_policy_path = False

        if get_exploit(sis_score, sis_visits) > exploit:
            max_exploit_path = False

        if float(sis_visits) > float(visits):
            max_visits_path = False

    max_policy_path = node.is_root() or (max_policy_path and node.get_ancestors()[0].max_policy_path)
    max_exploit_path = node.is_root() or (max_exploit_path and node.get_ancestors()[0].max_exploit_path)
    max_visits_path = node.is_root() or (max_visits_path and node.get_ancestors()[0].max_visits_path)

    node.add_features(
        max_policy_path=max_policy_path,
        max_exploit_path=max_exploit_path,
        max_visits_path=max_visits_path,
        faction=faction
    )

    primary_color, accent_color = {
        (False, False, False): ("#000000", "White"),  # Uncolored
        (False, False, True): ("#00006a", "#afafff"),  # Blue
        (False, True, False): ("#006a00", "#afffaf"),  # Green
        (False, True, True): ("#006a6a", "#afffff"),  # Cyan
        (True, False, False): ("#6a0000", "#ffafaf"),  # Red
        (True, False, True): ("#6a006a", "#ffafff"),  # Purple
        (True, True, False): ("#6a6a00", "#ffffaf"),  # Yellow
        (True, True, True): ("#303030", "#afafaf"),  # Gray (all)
    }[max_policy_path, max_exploit_path, max_visits_path]

    label_bg_color, label_fg_color = {
        -1: ("#006060", "#00ffff"),  # Undefined (Purple)
        0: (primary_color, accent_color),  # Player (White on Black)
        1: (accent_color, primary_color)  # Monster (Black on White)
    }[int(faction)]

    label = TextFace(action_info, "Verdana", 10, "0")
    label.background.color = label_bg_color
    label.fgcolor = label_fg_color
    label.border.width = 3
    label.rotation -= 90 if HORIZONTAL_STYLE else 0

    node.img_style["bgcolor"] = "White"
    node.img_style["vt_line_width"] = 2
    node.img_style["hz_line_width"] = 2
    node.img_style["vt_line_color"] = primary_color
    node.img_style["hz_line_color"] = primary_color
    label.border.color = primary_color
    label.margin_right = label.margin_top = label.margin_bottom = label.margin_left = 1

    if int(visits) == 0:
        node.img_style["vt_line_type"] = 2  # 0 solid, 1 dashed, 2 dotted
        node.img_style["hz_line_type"] = 2
        node.dist += 2.5

    # if node.get_ancestors() and not node.get_ancestors()[0].max_exploit_path and not node.get_ancestors()[
    #     0].max_visits_path:
    #     for child in node.get_children():
    #         child.detach()
    #     return

    add_face_to_node(label, node, 0, position="branch-right")

    # if int(visits) < 2500:
    #     for child in node.get_children():
    #         child.detach()


ts = TreeStyle()
ts.layout_fn = layout
ts.show_leaf_name = False
ts.show_scale = False
ts.branch_vertical_margin = 5
ts.rotation = 90 if HORIZONTAL_STYLE else 0

t = Tree(graph, format=1)
t.show(tree_style=ts)
